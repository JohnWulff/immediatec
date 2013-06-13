static const char load_c[] =
"@(#)$Id: load.c,v 1.57 2013/04/25 02:08:06 jw Exp $";
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
 *	load.c
 *	This module prepares the data structures for the run time
 *	system from the loaded data and calls the immediate C execution.
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<assert.h>
#ifndef LOAD
#error - must be compiled with LOAD defined to make a linkable library
#else
#ifdef TCP
#include	"tcpc.h"
#endif	/* TCP */
#include	"icc.h"

#define STS	1

extern const char	iC_ID[];

Gate		iClock = { -1, -CLK, CLCKL, 0, "iClock" };	/* active */

unsigned	errCount;
char *		iC_progname;		/* name of this executable */
char *		iC_vcd = NULL;
short		iC_debug = 0;
int		iC_micro = 0;
unsigned short	iC_osc_max = 0;		/* 0 during Initialisation, when no oscillations */
unsigned short	iC_osc_lim = MARKMAX;
unsigned short	iC_osc_flag = 0;

FILE *		iC_outFP;		/* listing file pointer */
FILE *		iC_errFP;		/* error file pointer */

static const char *	usage =
"Usage: %s [-"
#if YYDEBUG
"tx"
#endif	/* YYDEBUG */
"h]"
#ifdef TCP
#if YYDEBUG
"[ -m[m]]"
#endif	/* YYDEBUG */
"[ -s <server>][ -p <port>][ -i <instance_id>][ -v <file.vcd>]\n"
#endif	/* TCP */
"        [ -n <count>][ -d <debug>]\n"
#ifdef TCP
"        -s host ID      of server      (default '%s')\n"
"        -p service port of server      (default '%s')\n"
"        -i instance ID  of this client (default '%s'; 1 to %d numeric digits)\n"
"        -v <file.vcd>   output a .vcd and a .sav file for gtkwave\n"
#endif	/* TCP */
"        -n <count>      maximum oscillator count (default is %d, limit 15)\n"
"                        0 allows unlimited oscillations\n"
#if YYDEBUG
"        -d <debug>2000  display scan_cnt and link_cnt\n"
"                 +1000  do not trace non-active timers TX0.n\n"
"                  +400  exit after initialisation\n"
"                  +200  display oscillator info\n"
"                  +100  initialisation and run time trace\n"
#ifdef TCP
"            or     +20  trace only logic inputs as binary bytes\n"
#endif	/* TCP */
#else	/* YYDEBUG */
"        -d <debug> 400  exit after initialisation\n"
#endif	/* YYDEBUG */
"                   +40  load listing\n"
"                    +4  extra install debug info\n"
#if YYDEBUG
"                    +2  trace I/O receive buffer\n"
"                    +1  trace I/O send buffer\n"
"        -t              trace gate activity (equivalent to -d 1100)\n"
"                        can be toggled at run time by typing t\n"
#ifdef TCP
"        -m              microsecond timing info\n"
"        -mm             more microsecond timing (internal time base)\n"
"                        can be toggled at run time by typing m\n"
#endif	/* TCP */
#endif	/* YYDEBUG */
"        -h              this help text\n"
"                        typing q or ctrl-C quits run mode\n"
"compiled by:\n"
"%s\n"
"Copyright (C) 1985-2011 John E. Wulff     <immediateC@gmail.com>\n"
;

/********************************************************************
 *
 *	Constant compile typing data
 *
 *******************************************************************/

char *		iC_full_type[]  = { FULL_TYPE };
char *		iC_full_ftype[] = { FULL_FTYPE };
unsigned char	iC_types[]      = { TYPES };
unsigned char	iC_ctypes[]     = { CTYPES };
unsigned char	iC_ftypes[]     = { FTYPES };
char		iC_os[]         = OPS;
char		iC_fos[]        = FOPS;
char *		iC_useText[4]	= { USE_TEXT };

Gate *		iC_TX0p;		/* pointer to bit System byte Gates */

/********************************************************************
 *
 *	Error message routine for errors in loading application
 *
 *******************************************************************/

static void
inError(int line, Gate * op, Gate * gp, const char* errMsg)
{
    fprintf(stderr,
	"ERROR %s: line %d: op = %s gp = %s; %s\n",
	    __FILE__, line, op->gt_ids, gp ? gp->gt_ids : "'null'", errMsg);
    errCount++;
} /* inError */

/********************************************************************
 *
 *	Main for the whole application
 *
 *******************************************************************/

int
main(
    int		argc,
    char **	argv)
{
    Gate *	op;
    Gate *	gp;
    Gate **	sTable;			/* pointer to dynamic array */
    Gate **	sTend;			/* end of dynamic array */
    Gate ***	oppp;
    Gate **	opp;
    Gate **	lp;
    Gate **	slp;
    Gate **	tlp;
    Gate *	tgp;
    Gate *	ttgp;
    Gate *	e_list = 0;
    unsigned	inversion;
    unsigned	val = 0;
    unsigned	link_count = 0;
    Gate **	fp;
    Gate **	ifp;
    int		i;
    unsigned	df = 0;
    char *	cp;
    int		eLen;
    char	iqt[2];			/* single char buffer - space for 0 terminator */
    char	xbwl[2];		/* single char buffer - space for 0 terminator */
    int		byte;
    int		bit;
    char	tail[8];		/* compiler generated suffix _123456 max */
    char	eBuf[ESIZE];

    /* Process the arguments */
    if ((iC_progname = strrchr(*argv, '/')) == NULL
#ifdef	WIN32
	&& (iC_progname = strrchr(*argv, '\\')) == NULL
#endif	/* WIN32 */
    ) {
	iC_progname = *argv;
    } else {
	iC_progname++;			/*  path has been stripped */
    }
#ifdef TCP
    iC_iccNM = iC_emalloc(strlen(iC_progname)+INSTSIZE+2);	/* +2 for '-...\0' */
    strcpy(iC_iccNM, iC_progname);
#ifdef	WIN32
    if ((cp = strrchr(iC_iccNM, '.')) != 0 &&
	cp[1] == 'e' &&
	cp[2] == 'x' &&
	cp[3] == 'e' &&
	cp[4] == '\0') {
	*cp = '\0';			/* terminate at trailing extension .exe */
    }
#endif	/* WIN32 */
#endif	/* TCP */
    iC_outFP = stdout;			/* listing file pointer */
    iC_errFP = stderr;			/* error file pointer */
    while (--argc > 0) {
	if (**++argv == '-') {
	    ++*argv;
	    do {
		switch (**argv) {
#ifdef TCP
		case 's':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    iC_hostNM = *argv;
		    goto break2;
		case 'p':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    iC_portNM = *argv;
		    goto break2;
		case 'i':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if ((eLen = strlen(*argv)) > INSTSIZE ||
			eLen != strspn(*argv, "0123456789")) {
			fprintf(stderr, "WARNING '-i %s' is non numeric or longer than %d characters - ignored\n",
			    *argv, INSTSIZE);
		    } else {
			iC_iidNM = iC_emalloc(INSTSIZE+1);	/* +1 for '\0' */
			strncpy(iC_iidNM, *argv, INSTSIZE);
			snprintf(iC_iccNM + strlen(iC_iccNM), INSTSIZE+2, "-%s", iC_iidNM);
		    }
		    goto break2;
		case 'v':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    iC_vcd = *argv;	/* output vcd dump file for gtkwave */
		    goto break2;
		case 'm':
		    iC_micro++;		/* microsecond info */
		    break;
#endif	/* TCP */
		case 'd':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    sscanf(*argv, "%o", &df);
		    iC_debug |= df;	/* short */
		    df &= 040;		/* load listing */
		    goto break2;
		case 't':
		    iC_debug |= 01100;	/* trace gate activity */
		    break;
		case 'n':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    iC_osc_lim = atoi(*argv);
		    if (iC_osc_lim > 15) goto error;
		    goto break2;
		default:
		    fprintf(stderr,
			"%s: unknown flag '%c'\n", iC_progname, **argv);
		case 'h':
		case '?':
		error:
		    fprintf(stderr, usage, iC_progname,
#ifdef TCP
		    iC_hostNM, iC_portNM, iC_iidNM, INSTSIZE,
#endif	/* TCP */
		    MARKMAX, iC_ID);
		    exit(-1);
		}
	    } while (*++*argv);
	    break2: ;
	} else {
	    goto error;
	}
    }
    iC_debug &= 03767;			/* allow only cases specified */

/********************************************************************
 *
 *	Scan iC_list[] for PASS 0. Each entry contains a Gate** to the
 *	Gates in each separately compiled module. These Gates are linked
 *	via gt_next in the compiled code for each module.
 *
 *	PASS 0
 *
 *	Count all nodes to allocate a symbol table array sTable which
 *	is sorted in INTERLUDE 0. sTable is used for linear scans for all
 *	later initialisation and for finding nodes by name for debugging.
 *	It is important to have one list for multiple sources, so ALIAS's
 *	in different sources are resolved in the early passes.
 *
 *	Transfer timer preset value in gt_mark to gt_old for run time.
 *	NCONST and INPW nodes have gt_val initialised to -1.
 *	Clear all other gt_val, gt_old and all gt_mark entries except
 *	ALIAS nodes which hold inversion flag in gt_mark.
 *
 *	Generate an INPW/TRAB Gate IXx for each first IXx.y of gt_ini -INPX
 *	Generate an INPW/TRAB Gate TXx for each first TXx.y of gt_ini -INPX
 *	Generate an INPB/OUTW Gate QXx for each first QXx.y_0 of gt_fni OUTX
 *	These new Gates are linked in an extra list, which can be scanned
 *	reasonably quickly for duplicates. At the end of Pass 0, the extra
 *	list is linked into the start of the first entry in iC_list[], so the
 *	newly generated Gates are also scanned in Pass 1 and 2 and then sorted.
 *
 *******************************************************************/

    if (df) { printf("PASS 0\n"); fflush(stdout); }
    for (oppp = iC_list; (opp = *oppp++) != 0; ) {
	for (op = *opp; op != 0; op = op->gt_next) {
	    val++;					/* count node */
	    op->gt_val = (op->gt_ini == -NCONST	|| op->gt_ini == -INPW) ? -1 : 0;
	    if (op->gt_ini != -ALIAS) {
		op->gt_old = (op->gt_fni == TIMRL) ? op->gt_mark : 0;
		if ((op->gt_ini != -ARNC && op->gt_ini != -LOGC) || op->gt_fni != UDFA) {
		    op->gt_mark = 0;
		}
		/********************************************************************
		 * handle bit I/O
		 *******************************************************************/
		if (op->gt_ini == -INPX || op->gt_fni == OUTX) {
		    strncpy(eBuf, op->gt_ids, ESIZE);
		    cp = strchr(eBuf, '.');
		    i = sscanf(eBuf, "%1[IQT]X%5d.%5d%7s", iqt, &byte, &bit, tail);
		    tgp = 0;				/* for correct inError message */
		    if (cp && bit < 8) {
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
			val++;				/* count node */
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
			    if (i != 3 || op->gt_rlist != 0 || strcmp(iqt, "Q") == 0) goto pass0Err;
			    tgp->gt_list[bit] = op;	/* pointer to bit Gate */
			    /* ###### no back link ####### */
			} else {			/* (op->gt_fni == OUTX) */
			    if (i != 4 || strcmp(tail, "_0") != 0 || strcmp(iqt, "Q") != 0) goto pass0Err;
			    op->gt_mark = iC_bitMask[bit];
			    op->gt_ptr = tgp;		/* direct pointer (union with gt_list) */
			    /* ###### no back link ####### */
			}
			tgp->gt_mark |= iC_bitMask[bit];	/* note used bits for registration */
		    } else {
		      pass0Err:
			inError(__LINE__, op, tgp, "PASS 0: invalid bit I/O Gate");
		    }
		}
	    }
	}
    }
    if (errCount) {
	exit(1);					/* pass 0 failed */
    }
    if (e_list) {
	for (tgp = e_list; (ttgp = tgp->gt_next) != 0; tgp = ttgp);
	tgp->gt_next = *iC_list[0];	/* must have a non-null entry - works even if not */
	*iC_list[0] = e_list;		/* now e_list is at the front of first iC_list entry */
    }

/********************************************************************
 *
 *	INTERLUDE 0
 *
 *	Allocate space for the symbol table array.
 *
 *	Enter iClock, iConst and each node in iC_list into symbol table sTable.
 *	At the end of this pass sTend will hold the end of the table.
 *
 *	The length of a pointer array for such a symbol table is
 *	sTend - sTable.
 *
 *	Sort the symbol table in order of gt_ids.
 *
 *******************************************************************/

    val += 2;						/* count iClock and iConst */
    sTable = sTend = (Gate **)calloc(val, sizeof(Gate *));	/* node* */

    *sTend++ = &iClock;				/* enter iClock into sTable */
    *sTend++ = &iConst;				/* enter iConst into sTable */

    for (oppp = iC_list; (opp = *oppp++) != 0; ) {
	for (op = *opp; op != 0; op = op->gt_next) {
	    *sTend++ = op;				/* enter node into sTable */
	}
    }

    if (val != (sTend - sTable)) {
	fprintf(stderr,
	    "\nERROR: %s: line %d: Gate allocation %d vs Symbol Table size %d\n",
	    __FILE__, __LINE__, val, sTend - sTable);
	exit(2);
    }

    qsort(sTable, sTend - sTable, sizeof(Gate*), (iC_fptr)iC_cmp_gt_ids);

/********************************************************************
 *
 *	All the activity lists for action nodes are already correctly
 *	compiled and do not need to be altered except for a link from
 *	the SH slave gate to the corresponding delay master in gt_rlist.
 *
 *	They are now also joined and sorted. Resolve all ALIAS'es now.
 *
 *	PASS 1
 *
 *	Determine how much space is required for the output activity
 *	lists for ARITH and GATE nodes.
 *
 *	For each ARN and LOGIC node op accumulate count of outputs in
 *	gp->gt_mark, where gp is each of the nodes which is input to op.
 *	Also accumulate total count in link_count and the input count
 *	in op->gt_val for a consistency check. For C parameter values and
 *	timer delay references (these are ARITH nodes) accumulate the count
 *	in gp->gt_old without also accumulating a link_count, since there is
 *	no link. If a C parameter is assigned, which means it has an input
 *	in the C code, accumulate that count in gt_val. Such a C parameter
 *	must be of type ARNC or LOGC, which is checked.
 *
 *	For each ARITH and GATE node op, which is not an ALIAS, count
 *	1 extra link for ARITH nodes and 2 extra links for GATE nodes.
 *	These are space for the activity list terminators.
 *
 *	Resolve inputs from ARITH and GATE ALIAS nodes and OUTW nodes,
 *	which may occurr because of multiple modules linked together.
 *	In particular adjust each member of an ARITH ALIAS chain, so that
 *	it points to the final input. This is needed for execution of
 *	cexe_n() functions, where inputs can only resolve 1 level of ALIAS.
 *
 *	After this pass the input lists contain no aliases.
 *
 *******************************************************************/

    if (df) { printf("PASS 1 - name gt_ini gt_fni: input list\n"); fflush(stdout); }
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	/********************************************************************
	 *  part A
	 *  arithmetic node or logical output
	 *******************************************************************/
	if (op->gt_ini == -ARN) {
	    if (df) printf(" %-8s%6s%7s:", op->gt_ids,
			iC_full_type[-op->gt_ini], iC_full_ftype[op->gt_fni]);
	    if ((lp = op->gt_rlist) == 0) {
		inError(__LINE__, op, 0, "PASS 1: arithmetic node with no input list");
		goto partB;
	    }
	    lp++;					/* skip function vector (may be 0) */
	    while ((gp = *lp++) != 0) {			/* for ARN scan 1 list */
		if (gp->gt_ini == -ALIAS) {		/* resolve arithmetic ALIAS */
		    tgp = gp;				/* remember start of ALIAS chain */
		    while (gp->gt_ini == -ALIAS) {
			if (df) printf("	%s@", gp->gt_ids);
			gp = gp->gt_rptr;		/* adjust */
		    }
		    tlp = lp - 1;
		    *tlp = gp;				/* swap in real input */
		    /* adjust ALIAS chain, so that each ALIAS points to real input */
		    while (tgp->gt_ini == -ALIAS) {
			ttgp = tgp->gt_rptr;
			tgp->gt_rptr = gp;		/* point to final input gate */
			tgp = ttgp;
		    }
		}
		if (df) printf("	%s,", gp->gt_ids);
		if (op->gt_val < PPGATESIZE) {
		    op->gt_val++;			/* count input */
		}
		if (gp->gt_fni == ARITH) {
		    gp->gt_mark++;			/* arithmetic output at gp */
		    link_count++;
		} else
		if (gp->gt_fni != OUTX) {
		    inError(__LINE__, op, gp, "PASS 1: arithmetic node points back to non ARITH");
		    goto partB;
		}
	    }
	} else
	/********************************************************************
	 *  logical node
	 *******************************************************************/
	if (op->gt_ini <= -MIN_GT && op->gt_ini > -MAX_GT &&
	    op->gt_mcnt == 0) {				/* leave out _f0_1 or immC array */
	    if (df) {
		if (op->gt_ini < 0) {
		    printf(" %-8s%6s%7s:", op->gt_ids,
			iC_full_type[-op->gt_ini], iC_full_ftype[op->gt_fni]);
		} else {
		    /* should no longer occurr because of above test */
		    printf("*** Warning: wrong gt_ini:");
		    printf(" %-8s%6d%7s:", op->gt_ids,
			op->gt_ini, iC_full_ftype[op->gt_fni]);
		}
	    }
	    if ((lp = op->gt_rlist) == 0) {
		inError(__LINE__, op, 0, "PASS 1: logic node with no input list");
		goto partB;
	    }
	    i = 1;					/* LOGIC nodes XOR, AND, OR or LATCH */
	    /* for LOGIC scan 2 lists with i = 1 and -1 */
	    do {
		while ((gp = *lp++) != 0) {
		    inversion = 0;			/* resolve logical ALIAS */
		    if (gp->gt_ini == -ALIAS) {
			while (gp->gt_ini == -ALIAS) {
			    if (df) printf("	%c%s@",		/* @ */
				inversion ? '~' : ' ', gp->gt_ids);
			    inversion ^= gp->gt_mark;
			    gp = gp->gt_rptr;
			}
			tlp = lp - 1;		/* destination */
			if (inversion) {
			    slp = tlp + i;		/* source */
			    if (i == 1) {
				while ((*tlp++ = *slp++) != 0);
				*tlp = gp;		/* swap in real input */
				lp--;
				continue;		/* counted as inverse */
			    }
			    while ((*tlp-- = *slp--) != 0);
			}
			*tlp = gp;			/* swap in real input */
		    }
		    if (df) printf("	%c%s,",
			((i >> 1) & 0x1) ^ inversion ? '~' : ' ', gp->gt_ids);
		    op->gt_val++;			/* count input */
		    if (gp->gt_fni == GATE) {
			gp->gt_mark++;		/* logic output at gp */
			link_count++;
		    } else {
			if (gp->gt_fni < MAX_FTY && gp->gt_ini > -MAX_LS) {
			    inError(__LINE__, op, gp, "PASS 1: logic node points to non GATE");
			} else {
			    inError(__LINE__, op, 0, "PASS 1: logic node points to non Gate struct");
			}
			fprintf(stderr,
			    "ERROR %s: line %d: ftype = %d type = %d ???\n",
			    __FILE__, __LINE__, gp->gt_fni, gp->gt_ini);
			goto failed;		/* to avoid memory access error */
		    }
		}
	    } while ((i = -i) != 1);
	} else {
	    /********************************************************************
	     *  remaining types not touched in part A
	     *******************************************************************/
	    if (df) printf(" %s\t\t\t\t\tlink count = %d\n", op->gt_ids, link_count);
	    goto partB;				/* rest not touched in in part A */
	}
      failed:
	if (df) printf("\t\tlink count = %d\n", link_count);
      partB:
	/********************************************************************
	 *  part B
	 *  count links
	 *******************************************************************/
	if (op->gt_ini != -ALIAS &&
	    (op->gt_fni == ARITH || op->gt_fni == GATE)) {
	    if (op->gt_list != 0) {
		inError(__LINE__, op, 0, "PASS 1: no forward list for ARITH or GATE");
	    }
	    link_count++;				/* 1 terminator for ARITH */
	    if (op->gt_fni == GATE) {
		link_count++;				/* 2 terminators for GATE */
	    }
	} else
	/********************************************************************
	 *  action nodes
	 *******************************************************************/
	if (op->gt_fni >= MIN_ACT && op->gt_fni < MAX_ACT) {
	    op->gt_mark++;				/* count self */
	    lp = op->gt_list;
	    if (op->gt_mcnt == 0) {			/* leave out _f0_1 or immC array */
		if (lp == 0 || (gp = *lp++) == 0) {
		    inError(__LINE__, op, 0, "PASS 1: no slave node or funct for action");
		} else
		if (op->gt_fni != F_SW && op->gt_fni != F_CF && op->gt_fni != F_CE) {
		    gp->gt_val++;			/* mark slave node */
		    if (op->gt_fni == D_SH) {
			if (gp->gt_rlist) {
			    inError(__LINE__, op, gp, "PASS 1: backlink for D_SH slave already installed");
			} else {
			    gp->gt_rptr = op;		/* back pointer to delay master */
			}
		    }
		}
		if ((gp = *lp++) == 0 || (gp->gt_fni != CLCKL && gp->gt_fni != TIMRL)) {
		    inError(__LINE__, op, 0, "PASS 1: action has no clock or timer");
		} else {
		    if (gp->gt_ini == -ALIAS) {		/* resolve clock/timer alias */
			if (df) printf("    resolve clock  >");
			while (gp->gt_ini == -ALIAS) {
			    if (df) printf("	%s@", gp->gt_ids);
			    gp = gp->gt_rptr;		/* adjust */
			}
			if (df) printf("	%c%s\n", iC_os[-gp->gt_ini], gp->gt_ids);
			tlp = lp - 1;
			*tlp = gp;			/* swap in real clock or timer */
		    }
		    gp->gt_mark++;			/* clock or timer node */
		    if (gp->gt_fni == TIMRL) {
			if ((gp = *lp++) == 0) {
			    inError(__LINE__, op, 0, "PASS 1: timed action has no delay");
			} else {
			    if (gp->gt_ini == -ALIAS) {	/* resolve arithmetic alias */
				if (df) printf("    resolve delay  >");
				while (gp->gt_ini == -ALIAS) {
				    if (df) printf("	%s@", gp->gt_ids);
				    gp = gp->gt_rptr;	/* adjust */
				}
				if (df) printf("	<%s\n", gp->gt_ids);
				tlp = lp - 1;
				*tlp = gp;		/* swap in real input */
			    }
			    gp->gt_old++;		/* timer delay */
			}
		    }
		}
	    }
	    if (op->gt_fni == F_SW || op->gt_fni == F_CF || op->gt_fni == F_CE) {
		if ((slp = op->gt_rlist) == 0) {	/* F_SW to F_CE */
		    inError(__LINE__, op, 0, "PASS 1: no condition for switch, if else action");
		} else
		if (lp < slp) {
		    /********************************************************************
		     * slp also delimits C parameter list starting at lp
		     * for every USE_COUNT parameters there is 1 use word at the end.
		     * Therefore divide by USE_COUNT + 1, because slp points past extra
		     * use word(s).
		     *******************************************************************/
		    Gate **		ulp;
		    Gate **		elp;
		    int			uc;
		    int			j;
		    unsigned int	useWord = 0	/* satisfies -Wall */;

		    uc = slp - lp;
		    assert(uc >= 2);			/* at least one pointer and one use word */
		    uc = (USE_COUNT + uc) / (USE_COUNT + 1);	/* # of use words */
		    ulp = elp = slp - uc;
		    j = USE_COUNT - 1;
		    while (lp < elp) {
			if (++j >= USE_COUNT) {
			    useWord = (unsigned int)(unsigned long)*ulp++;	/* next use word */
			    j = 0;			/* executed at least once - useWord is initialized */
			} else {
			    useWord >>= 2;		/* next set of useBits */
			}
			if ((gp = *lp++) == 0) {
			    inError(__LINE__, op, 0, "PASS 1: C parameter missing");
			} else {
			    if (gp->gt_ini == -ALIAS) {	/* resolve C parameter alias */
				if (df) printf("    resolve C param>");
				while (gp->gt_ini == -ALIAS) {
				    if (df) printf("	%s@", gp->gt_ids);
				    gp = gp->gt_rptr;	/* adjust */
				}
				if (df) printf("	<%s\n", gp->gt_ids);
				tlp = lp - 1;
				*tlp = gp;		/* swap in real input */
			    }
			    if (useWord & (VAR_USE >> USE_OFFSET)) {
				gp->gt_old++;		/* C function value parameter */
			    }
			    if (useWord & (VAR_ASSIGN >> USE_OFFSET)) {
				assert(gp->gt_ini == -ARNC || gp->gt_ini == -LOGC);
				gp->gt_val++;		/* C parameter is assigned */
			    }
			}
		    }
		    assert(ulp == slp);
		    lp = slp;				/* skip over use words */
		}
	    }
	}
    }
    if (df) printf(" link count = %d\n", link_count);
    if (errCount) {
	exit(1);					/* pass 1 failed */
    }

/********************************************************************
 *
 *	INTERLUDE 1
 *
 *	Allocate space for the output activity lists for ARITH and
 *	GATE nodes.
 *
 *******************************************************************/

    ifp = fp = (Gate **)calloc(link_count, sizeof(Gate *));	/* links */

/********************************************************************
 *
 *	PASS 2
 *
 *	Using the counts accumulated in op->gt_mark determine the
 *	position of the last terminator for ARITH and GATE nodes.
 *	store this value in op->gt_list and clear the last terminator.
 *
 *	Using op->gt_val check the node has inputs.
 *	Using op->gt_mark or op->gt_old check the node has outputs.
 *
 *	The length of a pointer array for such a symbol table is
 *	sTend - sTable.
 *
 *******************************************************************/

    if (df) { printf("PASS 2 - symbol table: name inputs outputs delay-references\n"); fflush(stdout); }
    /* iClock is sorted into Symbol table and does not need to be reported seperately */
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	if (op->gt_ini != -ALIAS) {
	    if (op->gt_mark == 0 && op == &iConst) {
		if (df) printf(" %-8s %3d %3d - DELETED\n", op->gt_ids, op->gt_val, op->gt_mark);
		for (lp = opp, tlp = lp + 1; tlp < sTend;) {
		    *lp++ = *tlp++;			/* delete iConst from S.T. */
		}
		link_count--;				/* terminator for iConst not needed */
		opp--;					/* neutralise opp++ in for loop */
		sTend = lp;				/* S.T. is shortened */
	    } else {
		if (df) {
		    printf(" %-8s %3d %3d", op->gt_ids, op->gt_val, op->gt_mark);
		    if (op->gt_old) {
#if INT_MAX == 32767 && defined (LONG16)
			printf(" %3ld\n", op->gt_old);	/* delay references */
#else	/* INT_MAX == 32767 && defined (LONG16) */
			printf(" %3d\n", op->gt_old);	/* delay references */
#endif	/* INT_MAX == 32767 && defined (LONG16) */
		    } else {
			printf("\n");
		    }
		}
		if (op->gt_val == 0 &&
		    op->gt_mcnt == 0 &&			/* leave out _f0_1 or immC array */
		    op->gt_ini != -ARNC &&
		    op->gt_ini != -LOGC &&
		    op->gt_ini != -INPB &&
		    op->gt_ini != -INPW &&
		    op->gt_ini != -INPX) {
		    fprintf(stderr, "WARNING '%s' has no input\n", op->gt_ids);
		    if (df) printf("*** Warning: '%s' has no input\n", op->gt_ids);
		}
		if (op->gt_mark == 0 &&
		    op->gt_old == 0 &&
		    op->gt_ini != -ARNC &&
		    op->gt_ini != -LOGC &&
		    op->gt_fni != TRAB &&
		    op->gt_fni != OUTW &&
		    op->gt_fni != OUTX &&
		    op != & iClock) {
		    fprintf(stderr, "WARNING '%s' has no output\n", op->gt_ids);
		    if (df) printf("*** Warning: '%s' has no output\n", op->gt_ids);
		}
		if (op->gt_fni == ARITH) {
		    op->gt_old = 0;		/* clear for run time after ARITH was delay */
		    fp += op->gt_mark;
		    *fp = 0;			/* last output terminator */
		    op->gt_list = fp++;
		} else
		if (op->gt_fni == GATE) {
		    fp += op->gt_mark + 1;
		    *fp = 0;			/* last output terminator */
		    op->gt_list = fp++;
		}
		if (op->gt_ini != -INPW && op->gt_fni != OUTW && op->gt_fni != OUTX &&
		    ((op->gt_ini != -ARNC && op->gt_ini != -LOGC) || op->gt_fni != UDFA)) {
		    op->gt_mark = 0;		/* must be cleared for run-time */
		}
	    }
	} else
	if (df) {
	    int inverse = 0;			/* print ALIAS in symbol table */
	    printf(" %s@	", op->gt_ids);
	    gp = op;
	    while (gp->gt_ini == -ALIAS) {
		if (gp->gt_fni == GATE) {
		    inverse ^= gp->gt_mark;	/* holds ALIAS inversion flag */
		}
		gp = gp->gt_rptr;		/* resolve ALIAS */
	    }
	    printf("%s%s\n", inverse ? "~" : " ", gp->gt_ids);
	}
    }

    if ((val = fp - ifp) != link_count) {
	fprintf(stderr,
	    "\nERROR: %s: line %d: link error %d vs link_count %d\n",
	    __FILE__, __LINE__, val, link_count);
	exit(2);
    }

/********************************************************************
 *
 *	PASS 3
 *
 *	Transfer LOGIC inverted inputs, storing them via gp->gt_list,
 *	which is pre-decremented. (same in PASS 4 and 5)
 *
 *	Since lists are now joined and sorted adjust remaining ALIAS's
 *	which arise in different sources. Do the same in PASS 5 for
 *	normal logic and arithmetic nodes and in PASS 6 for misc nodes.
 *
 *******************************************************************/

    if (df) { printf("PASS 3\n"); fflush(stdout); }
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	if (op->gt_ini <= -MIN_GT && op->gt_ini > -MAX_GT &&
	    op->gt_mcnt == 0) {			/* leave out _f0_1 or immC array */
	    lp = op->gt_rlist;			/* reverse or input list */
	    while (*lp++ != 0);			/* skip normal inputs */
	    while ((gp = *lp++) != 0) {
		if (op->gt_ini == -XOR && gp->gt_fni == GATE) {
		    gp->gt_fni = GATEX;		/* this source must handle XOR */
		}
		*(--(gp->gt_list)) = op;	/* transfer inverted */
	    }
	}
    }

/********************************************************************
 *
 *	PASS 4
 *
 *	Insert first terminator for GATE nodes via op->gt_list.
 *
 *******************************************************************/

    if (df) { printf("PASS 4\n"); fflush(stdout); }
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	if (op->gt_ini != -ALIAS && (op->gt_fni == GATE || op->gt_fni == GATEX)) {
	    *(--(op->gt_list)) = 0;		/* GATE normal terminator */
	}
    }

/********************************************************************
 *
 *	PASS 5
 *
 *	Transfer inputs for ARN and normal inputs for LOGIC nodes.
 *	At the end of this pass, the gt_list entries all point to
 *	the start of their respective activity lists.
 *
 *******************************************************************/

    if (df) { printf("PASS 5\n"); fflush(stdout); }
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	if (op->gt_ini == -ARN ||
	    (op->gt_ini <= -MIN_GT && op->gt_ini > -MAX_GT &&
	    op->gt_mcnt == 0)) {		/* leave out _f0_1 or immC array */
	    lp = op->gt_rlist + (op->gt_ini == -ARN);
	    while ((gp = *lp++) != 0) {
		if (op->gt_ini == -XOR && gp->gt_fni == GATE) {
		    gp->gt_fni = GATEX;		/* this source must handle XOR */
		}
		*(--(gp->gt_list)) = op;	/* transfer normal */
	    }
	}
    }

/********************************************************************
 *
 *	PASS 6
 *
 *	Do a full consistency check on pre-compiled structures in
 *	case somebody messed around with the generated intermediate
 *	C files or the iC compiler is in error (!!)
 *
 *******************************************************************/

    if (df) { printf("PASS 6 - name gt_ini gt_fni: output list\n"); fflush(stdout); }
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	if (op->gt_ini != -ALIAS) {
	    if (df) {
		if (op->gt_ini < 0) {
		    printf(" %-8s%6s%7s:", op->gt_ids,
			iC_full_type[-op->gt_ini], iC_full_ftype[op->gt_fni]);
		} else {
		    printf(" %-8s%6d%7s:", op->gt_ids,
			op->gt_ini, iC_full_ftype[op->gt_fni]);
		}
	    }
	    if ((op->gt_ini == -ARNC || op->gt_ini == -LOGC) && op->gt_fni == UDFA) {
		int	n = 0;				/* immC array */
		if ((lp = op->gt_rlist) == 0) {
		    inError(__LINE__, op, 0, "PASS 6: immC array node with no output list");
		}
		while ((gp = *lp++) != 0) {		/* scan array initialisers and check count */
		    if (df) {
			printf("\t[%d] %s", n, gp->gt_ids);
		    }
		    n++;
		}
		if (n != op->gt_mark) {			/* use gt_mark, which is short to allow arrays up to size 32767 */
		    inError(__LINE__, op, 0, "PASS 6: immC array has incorrect number of initialisers");
		}
		/* gt_mark gets clobberd by oscillator marking - transferred to gt_old in INI Pass 3 */
	    } else {
		if (op->gt_ini == -ARN) {
		    if ((lp = op->gt_rlist) == 0) {
			inError(__LINE__, op, 0, "PASS 6: arithmetic node with no input list");
		    }
		    gp = *lp++;				/* skip function vector (may be 0) */
#ifdef HEXADDRESS
		    if (df) printf("	0x%lx()", (long)gp);	/* cexe_n */
#else	/* HEXADDRESS */
		    if (df) printf("	0x0()");		/* dummy */
#endif	/* HEXADDRESS */
		    while ((gp = *lp++) != 0) {		/* for ARN scan 1 input list */
			if (gp->gt_ini == -ALIAS) {		/* arithmetic ALIAS */
			    inError(__LINE__, op, gp, "PASS 6: input int ALIAS not resolved");
			}
		    }
		} else
		/********************************************************************
		 *  arithmetic or logical input linkage to physical I/O (mainly display)
		 *******************************************************************/
		if (op->gt_ini == -INPW) {
		    if (sscanf(op->gt_ids, "%1[IT]%1[XBWL]%5d%7s",
			    iqt, xbwl, &byte, tail) == 3) {
			switch (iqt[0]) {
			case 'T':
			    if (byte != 0) goto inErr;	/* TXD must be 1 */
			    iC_TX0p = op;			/* forward input link */
			    /* fall through */
			case 'I':
			    break;
			default:
			    goto inErr;
			}
		    } else {
		      inErr:
			inError(__LINE__, op, 0, "PASS 6: no valid input word definition");
		    }
		}
		if (op->gt_fni == UDFA) {
		    inError(__LINE__, op, 0, "PASS 6: has undefined output type UDFA");
		} else
		if (op->gt_fni < MIN_ACT) {
		    tlp = lp = op->gt_list;		/* ARITH or GATE */
		    while (*tlp++ != 0);		/* skip inputs */
		    /* sort gate list */
		    qsort(lp, tlp - lp - 1, sizeof(Gate*), (iC_fptr)iC_cmp_gt_ids);
		    if (df) {
			while ((gp = *lp++) != 0) {
			    if (gp->gt_ini == -ALIAS) {	/* resolve bit/int alias */
				inError(__LINE__, op, gp, "PASS 6: bit or int ALIAS not resolved");
				printf("	@%s,", gp->gt_ids);
			    } else {
				printf("	%s,", gp->gt_ids);
			    }
			}
		    }
		    if (op->gt_fni == GATE || op->gt_fni == GATEX) {
			lp = tlp;
			while (*tlp++ != 0);		/* skip inputs */
			/* sort gate list */
			qsort(lp, tlp - lp - 1, sizeof(Gate*), (iC_fptr)iC_cmp_gt_ids);
			if (df) {
			    while ((gp = *lp++) != 0) {
				if (gp->gt_ini == -ALIAS) {	/* resolve inverted bit alias */
				    inError(__LINE__, op, gp, "PASS 6: inverted bit ALIAS not resolved");
				    printf("	@~%s,", gp->gt_ids);
				} else {
				    printf("	~%s,", gp->gt_ids);
				}
			    }
			}
		    }
		} else
		if (op->gt_fni < MAX_ACT) {
		    lp = op->gt_list;				/* RI_BIT to TIMR */
		    i = 0;						/* offset for _f0_1 */
		    if (op->gt_mcnt == 0) {				/* leave out _f0_1 or immC array */
			if (lp == 0 || (gp = *lp++) == 0) {
			    inError(__LINE__, op, 0, "PASS 6: no slave for master action RI_BIT to TIMR");
			} else
			if (op->gt_fni != F_SW && op->gt_fni != F_CF && op->gt_fni != F_CE) {
			    if (df) printf("	%s,", gp->gt_ids);
			} else {
#ifdef HEXADDRESS
			    if (df) printf("	0x%lx()", (long)gp);	/* cexe_n */
#else	/* HEXADDRESS */
			    if (df) printf("	0x0(),");		/* dummy */
#endif	/* HEXADDRESS */
			}
			i = 2;					/* offset for clock */
			if ((gp = *lp++) == 0) {
			    inError(__LINE__, op, 0, "PASS 6: no clock or timer for master action");
			} else
			if (gp->gt_fni != CLCKL && gp->gt_fni != TIMRL) {
			    inError(__LINE__, op, gp, "PASS 6: strange clock or timer");
			} else {
			    if (gp->gt_ini == -ALIAS) {		/* resolve clock/timer alias */
				inError(__LINE__, op, gp, "PASS 6: clock or timer ALIAS not resolved");
			    }
			    if ((gp->gt_fni != CLCKL || gp->gt_ini != -CLK) &&
				(gp->gt_fni != TIMRL || gp->gt_ini != -TIM)) {
				inError(__LINE__, op, gp, "PASS 6: clock or timer master does not match slave");
			    }
			    if (df) printf("	%c%s,", iC_os[-gp->gt_ini], gp->gt_ids);
			    if (gp->gt_fni == TIMRL) {
				if ((gp = *lp++) == 0) {
				    inError(__LINE__, op, 0, "PASS 6: timed action has no delay");
				} else {
				    if (gp->gt_ini == -ALIAS) {	/* resolve arithmetic alias */
					inError(__LINE__, op, gp, "PASS 6: arithmetic delay ALIAS not resolved");
				    }
				    if (df) printf("	<%s,", gp->gt_ids);
				}
				i = 3;				/* offset for timer */
			    }
			}
		    }
		    if (op->gt_fni == F_SW || op->gt_fni == F_CF || op->gt_fni == F_CE) {	/* includes literal block _f0_1 */
			if ((slp = op->gt_rlist) == 0) {	/* F_SW to F_CE */
			    inError(__LINE__, op, 0, "PASS 6: no condition for switch, if else action");
			} else
			if (lp < slp) {
			    /********************************************************************
			     * slp also delimits C parameter list starting at lp
			     * for every USE_COUNT parameters there is 1 use word at the end.
			     * Therefore divide by USE_COUNT + 1, because slp points past extra
			     * use word(s).
			     *******************************************************************/
			    Gate **		ulp;
			    Gate **		elp;
			    int		uc;
			    int		j;
			    unsigned int	useWord = 0	/* satisfies -Wall */;

			    uc = slp - lp;
			    assert(uc >= 2);			/* at least one pointer and one use word */
			    uc = (USE_COUNT + uc) / (USE_COUNT + 1);	/* # of use words */
			    ulp = elp = slp - uc;
			    j = USE_COUNT - 1;
			    while (lp < elp) {
				if (++j >= USE_COUNT) {
				    useWord = (unsigned int)(unsigned long)*ulp++;	/* next use word */
				    j = 0;			/* executed at least once - useWord is initialized */
				} else {
				    useWord >>= 2;			/* next set of useBits */
				}
				if ((gp = *lp++) == 0) {
				    inError(__LINE__, op, 0, "PASS 6: C parameter missing");
				} else {
				    if (gp->gt_ini == -ALIAS) {	/* arithmetic alias */
					inError(__LINE__, op, gp, "PASS 6: C parameter ALIAS not resolved");
				    }
				    if (df) printf("\tC%d %s %s,", i++, gp->gt_ids,
					iC_useText[useWord & (USE_MASK >> USE_OFFSET)]);
				}
			    }
			    lp = slp;				/* skip over use words */
			}
		    }
		} else
		/********************************************************************
		 *  arithmetic or logical output linkage to physical I/O
		 *******************************************************************/
		if (op->gt_fni == OUTW) {
		    if ((i = sscanf(op->gt_ids, "Q%1[XBWL]%5d%7s", xbwl, &byte, tail)) >= 2) {
			switch (xbwl[0]) {
			case 'X':
			    if (i > 2) goto outErr;		/* no tail _0 allowed for QXn */
			    break;				/* op->gt_mark set to used bits 0x01 to 0x80 in Pass 0 */
			case 'B':
			    if (i != 3 || strcmp(tail, "_0") != 0) goto outErr;
			    op->gt_out = 0;			/* initial default value (gt_rlist no longer needed) */
			    op->gt_mark = B_WIDTH;
			    break;
			case 'W':
			    if (i != 3 || strcmp(tail, "_0") != 0) goto outErr;
			    op->gt_out = 0;			/* initial default value (gt_rlist no longer needed) */
			    op->gt_mark = W_WIDTH;
			    break;
#if INT_MAX != 32767 || defined (LONG16)
			case 'L':
			    if (i != 3 || strcmp(tail, "_0") != 0) goto outErr;
			    op->gt_out = 0;			/* initial default value (gt_rlist no longer needed) */
			    op->gt_mark = L_WIDTH;
			    break;
#endif	/* INT_MAX != 32767 || defined (LONG16) */
			default:
			    goto outErr;
			}
			if (df) printf("	0x%02x", op->gt_mark);
		    } else {
		      outErr:
			inError(__LINE__, op, 0, "PASS 6: no valid output word definition");
		    }
		} else
		if (op->gt_fni < CLCKL) {
		    if (df) {
			if (op->gt_fni == OUTX) {
			    gp = op->gt_ptr;		/* OUTX */
			    printf("	%s	0x%02x", gp->gt_ids, op->gt_mark);
			}
		    }
		} else
		if (op->gt_fni <= TIMRL) {
		    if (op->gt_list != 0) {
			inError(__LINE__, op, 0, "PASS 6: clock or timer with initial forward list");
		    }
		    if (op->gt_fni == TIMRL && op->gt_old > 0) {
#if INT_MAX == 32767 && defined (LONG16)
			if (df) printf("	(%ld)", op->gt_old);
#else	/* INT_MAX == 32767 && defined (LONG16) */
			if (df) printf("	(%d)", op->gt_old);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
		    }
		} else {
		    inError(__LINE__, op, 0, "PASS 6: unknown output type (ftype)");
		}
	    }
	    if (df) printf("\n");
	} else
	if (df) {
	    int inverse = 0;				/* print ALIAS in symbol table */
	    gp = op;
	    printf(" %-8s%6s%7s:", op->gt_ids,
		iC_full_type[-op->gt_ini], iC_full_ftype[op->gt_fni]);
	    while (gp->gt_ini == -ALIAS) {
		if (gp->gt_fni == GATE || gp->gt_fni == GATEX) {
		    inverse ^= gp->gt_mark;		/* holds ALIAS inversion flag */
		}
		gp = gp->gt_rptr;			/* resolve ALIAS */
	    }
	    printf("	%s%s\n", inverse ? "~" : "", gp->gt_ids);
	}
    }
    if (errCount) {
	exit(6);					/* pass 6 failed */
    }
    iC_icc(sTable, sTend);				/* execute load object */
    /********************************************************************
     * never returns - exits via iC_quit()
     *******************************************************************/
    return 0;
} /* main */
#endif	/* LOAD */

static const char load_c[] =
"@(#)$Id: load.c,v 1.45 2005/01/16 15:24:57 jw Exp $";
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
 *	load.c
 *	This module prepares the data structures for the run time
 *	system from the loaded data and calls the icc execution.
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#ifndef LOAD
#error - must be compiled with LOAD defined to make a linkable library
#else
#ifdef TCP
#include	"tcpc.h"
#endif
#include	"icg.h"
#include	"icc.h"

#define STS	1
#define ESIZE	512

extern const char	SC_ID[];

typedef int (*fptr)(const void*, const void*);

Gate		iClock = { -1, -CLK, CLCKL, 0, "iClock" };	/* active */
Gate **		sTable;			/* pointer to dynamic array */
Gate **		sTend;			/* end of dynamic array */

unsigned	errCount;
const char *	progname;		/* name of this executable */
short		debug = 0;
int		micro = 0;
unsigned short	xflag;
unsigned short	osc_max = MARKMAX;
unsigned short	osc_lim = MARKMAX;

FILE *		outFP;			/* listing file pointer */
FILE *		errFP;			/* error file pointer */

static char	eBuf[ESIZE];

static const char *	usage =
"USAGE: %s [-"
#if YYDEBUG
"t"
#endif
"xh]"
#ifdef TCP
" [-m[m]] [-s <server>] [-p <port>] [-i <instance_id>]\n"
#endif
" [-n<count>] [-d<debug>]\n"
#ifdef TCP
"        -s host ID of server      (default '%s')\n"
"        -p service port of server (default '%s')\n"
"        -i instance id of this client (default '%s'; 1 to %d numeric digits)\n"
#endif
"        -n <count>      maximum oscilator count (default is %d, limit 15)\n"
#if YYDEBUG
"        -d <debug>2000  display scan_cnt and link_cnt\n"
"                 +1000  do not trace non-active timers TX0.n\n"
"                  +400  exit after initialisation\n"
"                  +200  display oscillator info\n"
"                  +100  initialisation and run time trace\n"
#else
"        -d <debug> 400  exit after initialisation\n"
#endif
"                   +40  load listing\n"
"                    +4  extra install debug info\n"
#if YYDEBUG
"                    +2  trace I/O receive buffer\n"
"                    +1  trace I/O send buffer\n"
"        -t              trace debug (equivalent to -d 100)\n"
"                        can be toggled at run time by typing t\n"
#endif
#ifdef TCP
"        -m              microsecond timing info\n"
"        -mm             more microsecond timing (internal time base)\n"
"                        can be toggled at run time by typing m\n"
#endif
"        -x              arithmetic info in hexadecimal (default decimal)\n"
"                        can be changed at run time by typing x or d\n"
"        -h              this help text\n"
"                        typing q or ctrl-C quits run mode\n"
"compiled by:\n"
"%s\n"
"Copyright (C) 1985-2005 John E. Wulff     <john@je-wulff.de>\n"
;

/********************************************************************
 *
 *	Constant compile typing data
 *
 *******************************************************************/

char *		full_type[]  = { FULL_TYPE };
char *		full_ftype[] = { FULL_FTYPE };
unsigned char	types[]      = { TYPES };
unsigned char	ctypes[]     = { CTYPES };
unsigned char	ftypes[]     = { FTYPES };
char		os[]         = OPS;
char		fos[]        = FOPS;

unsigned char	bitMask[]    = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,	/* 0 1 2 3 4 5 6 7 */
};

/********************************************************************
 *
 *	I/O arrays also used at compile time
 *
 *******************************************************************/

Gate *		IX_[IXD];		/* pointers to bit Input byte Gates */
Gate *		IB_[IXD];		/* pointers to byte Input Gates */
Gate *		IW_[IXD];		/* pointers to word Input Gates */
#if INT_MAX != 32767 || defined (LONG16)
Gate *		IL_[IXD];		/* pointers to long Input Gates */
#endif
Gate *		TX_[TXD];		/* pointers to bit System byte Gates */
unsigned char	QX_[IXD];		/* Output bit field slots */
char		QT_[IXD];		/* Output type of slots */

/********************************************************************
 *
 *	Compare gt_ids in two Gates support of qsort()
 *
 *******************************************************************/

static int
cmp_gt_ids( const Gate ** a, const Gate ** b)
{
    return( strcmp((*a)->gt_ids, (*b)->gt_ids) );
}

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
    Gate ***	oppp;
    Gate **	opp;
    Gate **	lp;
    Gate **	slp;
    Gate **	tlp;
    Gate *	tgp;
    Gate *	ttgp;
    Gate *	e_list = 0;
    unsigned	inversion;
    unsigned	val;
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

    /* Process the arguments */
    if ((progname = strrchr(*argv, '/')) == NULL) {
	progname = *argv;
    } else {
	progname++;		/*  path has been stripped */
    }
#ifdef TCP
    iccNM = emalloc(strlen(progname)+INSTSIZE+2);	/* +2 for '-...\0' */
    strcpy(iccNM, progname);
#endif
    outFP = stdout;		/* listing file pointer */
    errFP = stderr;		/* error file pointer */
    while (--argc > 0) {
	if (**++argv == '-') {
	    ++*argv;
	    do {
		switch (**argv) {
#ifdef TCP
		case 's':
		    if (! *++*argv) { --argc, ++argv; }
		    hostNM = *argv;
		    goto break2;
		case 'p':
		    if (! *++*argv) { --argc, ++argv; }
		    portNM = *argv;
		    goto break2;
		case 'i':
		    if (! *++*argv) { --argc, ++argv; }
		    if ((eLen = strlen(*argv)) > INSTSIZE ||
			eLen != strspn(*argv, "0123456789")) {
			fprintf(stderr, "WARNING '-i %s' is non numeric or longer than %d characters - ignored\n",
			    *argv, INSTSIZE);
		    } else {
			iidNM = emalloc(INSTSIZE+1);	/* +1 for '\0' */
			strncpy(iidNM, *argv, INSTSIZE);
			snprintf(iccNM + strlen(iccNM), INSTSIZE+2, "-%s", iidNM);
		    }
		    goto break2;
		case 'm':
		    micro++;		/* microsecond info */
		    break;
#endif
		case 'd':
		    if (! *++*argv) { --argc, ++argv; }
		    sscanf(*argv, "%o", &df);
		    debug |= df;	/* short */
		    df &= 040;		/* load listing */
		    goto break2;
		case 't':
		    debug |= 0100;	/* trace */
		    break;
		case 'n':
		    if (! *++*argv) { --argc, ++argv; }
		    osc_lim = osc_max = atoi(*argv);
		    if (osc_max > 15) goto error;
		    goto break2;
		case 'x':
		    xflag = 1;		/* start with hexadecimal output */
		    break;
		default:
		    fprintf(stderr,
			"%s: unknown flag '%c'\n", progname, **argv);
		case 'h':
		case '?':
		error:
		    fprintf(stderr, usage, progname,
#ifdef TCP
		    hostNM, portNM, iidNM, INSTSIZE,
#endif
		    MARKMAX, SC_ID);
		    exit(-1);
		}
	    } while (*++*argv);
	    break2: ;
	} else {
	    goto error;
	}
    }
    debug &= 03747;			/* allow only cases specified */

/********************************************************************
 *
 *	Scan i_list[] for PASS 0 to 2. Each entry contains a Gate** to the
 *	Gates in each seperately compiled module. These Gates are linked
 *	via gt_next in the compiled code for each module.
 *
 *	PASS 0
 *
 *	Transfer timer preset value in gt_mark to gt_old for run time.
 *	NCONST and INPW nodes have gt_val initialised to -1.
 *	Clear all other gt_val and all gt_mark entries except
 *	ALIAS nodes which hold inversion flag in gt_mark.
 *
 *	Generate an INPW/TRAB Gate IXx for each first IXx.y of gt_ini -INPX
 *	Generate an INPW/TRAB Gate TXx for each first TXx.y of gt_ini -INPX
 *	Generate an INPB/OUTW Gate QXx for each first QXx.y_0 of gt_fni OUTX
 *	These new Gates are linked in an extra list, which can be scanned
 *	reasonably quickly for duplicates. At the end of Pass 0, the extra
 *	list is linked into the start of the first entry in i_list[], so the
 *	newly generated Gates are also scanned in Pass 1 and 2 and then sorted.
 *
 *******************************************************************/

    if (df) { printf("PASS 0\n"); fflush(stdout); }
    for (oppp = i_list; (opp = *oppp++) != 0; ) {
	for (op = *opp; op != 0; op = op->gt_next) {
	    op->gt_val = (op->gt_ini == -NCONST	|| op->gt_ini == -INPW) ? -1 : 0;
	    if (op->gt_ini != -ALIAS) {
		op->gt_old = (op->gt_fni == TIMRL) ? op->gt_mark : 0;
		op->gt_mark = 0;
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
			tgp = (Gate *) emalloc(sizeof(Gate));
			tgp->gt_ids = emalloc(eLen);	/* all bytes 0 */
			strncpy(tgp->gt_ids, eBuf, eLen);
			tgp->gt_next = e_list;		/* link rest of e_list to new Gate */
			e_list = tgp;			/* new Gate at front for speed */
			if (op->gt_ini == -INPX) {
			    tgp->gt_ini = -INPW;
			    tgp->gt_fni = TRAB;
			    tgp->gt_list = (Gate **) emalloc(8 * sizeof(Gate *));
			} else {			/* (op->gt_fni == OUTX) */
			    tgp->gt_ini = -INPB;
			    tgp->gt_fni = OUTW;
			}
		      linkIO:
			if (op->gt_ini == -INPX) {
			    if (i != 3 || op->gt_rlist != 0 ||
				(strcmp(iqt, "I") && strcmp(iqt, "T"))) goto pass0Err;
			    tgp->gt_list[bit] = op;	/* pointer to bit Gate */
			    /* ###### no back link ####### */
			} else {			/* (op->gt_fni == OUTX) */
			    if (i != 4 || strcmp(tail, "_0") || strcmp(iqt, "Q")) goto pass0Err;
			    op->gt_mark = bitMask[bit];
			    op->gt_list = (Gate**)tgp;	/* pointer directly in gt_list */
			    /* ###### no back link ####### */
			}
		    } else {
		      pass0Err:
			inError(__LINE__, op, tgp, "PASS 0: invalid bit I/O Gate");
		    }
		}
	    }
	}
    }
    if (errCount) {
	exit(1);		/* pass 0 failed */
    }
    if (e_list) {
	for (tgp = e_list; (ttgp = tgp->gt_next) != 0; tgp = ttgp);
	tgp->gt_next = *i_list[0];	/* must have a non-null entry - works even if not */
	*i_list[0] = e_list;		/* now e_list is at the front of first i_list entry */
    }

/********************************************************************
 *
 *	All the activity lists for action nodes are already correctly
 *	compiled and do not need to be altered except for a link from
 *	the SH slave gate to the corresponding delay master in gt_rlist.
 *
 *	PASS 1
 *
 *	Count all nodes to allocate a symbol table array sTable which
 *	is later sorted. sTable is used for linear scans for all later
 *	initialisation and for finding nodes by name for debugging.
 *
 *	Determine how much space is required for the output activity
 *	lists for ARITH and GATE nodes.
 *
 *	For each ARN and LOGIC node op accumulate count of outputs in
 *	gp->gt_mark, where gp is each of the nodes which is input to op.
 *	Also accumulate total count in link_count and the input count
 *	in op->gt_val for a consistency check. For timer delay references
 *	accumulate the count in gp->gt_old (this is an ARITH node) without
 *	also accumulating a link_count, since there is no link.
 *
 *	For each ARITH and GATE node op, which is not an ALIAS, count
 *	1 extra link for ARITH nodes and 2 extra links for GATE nodes.
 *	These are space for the activity list terminators.
 *
 *	Resolve inputs from ARITH and GATE ALIAS nodes and OUTW nodes,
 *	which may occurr because of multiple modules linked together.
 *	In particular adjust each member of an ARITH ALIAS chain, so that
 *	it points to the final input. This is needed for execution of
 *	cexe_n() functions, where inputs can only resove 1 level of ALIAS.
 *
 *	After this pass the input lists contain no aliases.
 *
 *******************************************************************/

    if (df) { printf("PASS 1 - name gt_ini gt_fni: input list\n"); fflush(stdout); }
    val = 1;					/* count iClock */
    for (oppp = i_list; (opp = *oppp++) != 0; ) {
	for (op = *opp; op != 0; op = op->gt_next) {
	    val++;				/* count node */
	    /********************************************************************
	     *  part A
	     *  arithmetic node or logical output
	     *******************************************************************/
	    if (op->gt_ini == -ARN) {
		if (df) printf(" %-8s%6s%7s:", op->gt_ids,
			    full_type[-op->gt_ini], full_ftype[op->gt_fni]);
		if ((lp = op->gt_rlist) == 0) {
		    inError(__LINE__, op, 0, "PASS 1: arithmetic node with no forward list");
		    goto partB;
		}
		if (op->gt_ini == -ARN) {
		    lp++;			/* skip function vector */
		}
		while ((gp = *lp++) != 0) {	/* for ARN scan 1 list */
		    if (gp->gt_ini == -ALIAS) {	/* resolve arithmetic ALIAS */
			tgp = gp;		/* remember start of ALIAS chain */
			while (gp->gt_ini == -ALIAS) {
			    if (df) printf("	%s@", gp->gt_ids);
			    gp = (Gate*)gp->gt_rlist;	/* adjust */
			}
			tlp = lp - 1;
			*tlp = gp;		/* swap in real input */
			/* adjust ALIAS chain, so that each ALIAS points to real input */
			while (tgp->gt_ini == -ALIAS) {
			    ttgp = (Gate*)tgp->gt_rlist;
			    tgp->gt_rlist = (Gate**)gp;	/* point to final input gate */
			    tgp = ttgp;
			}
		    }
		    if (df) printf("	%s,", gp->gt_ids);
		    op->gt_val++;		/* count input */
		    if (gp->gt_fni == ARITH) {
			gp->gt_mark++;		/* arithmetic output at gp */
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
	    if (op->gt_ini > 0) {
		if (df) printf(" %-8s%6d%7s:", op->gt_ids,
			    op->gt_ini, full_ftype[op->gt_fni]);
		if ((lp = op->gt_rlist) == 0) {
		    inError(__LINE__, op, 0, "PASS 1: logic node with no forward list");
		    goto partB;
		}
		i = 1;			/* LOGIC nodes AND, OR or LATCH */
		/* for LOGIC scan 2 lists with i = 1 and -1 */
		do {
		    while ((gp = *lp++) != 0) {
			inversion = 0;		/* resolve logical ALIAS */
			if (gp->gt_ini == -ALIAS) {
			    while (gp->gt_ini == -ALIAS) {
				if (df) printf("	%c%s@",		/* @ */
				    inversion ? '~' : ' ', gp->gt_ids);
				inversion ^= gp->gt_mark;
				gp = (Gate*)gp->gt_rlist;
			    }
			    tlp = lp - 1;	/* destination */
			    if (inversion) {
				slp = tlp + i;	/* source */
				if (i == 1) {
				    while ((*tlp++ = *slp++) != 0);
				    *tlp = gp;	/* swap in real input */
				    lp--;
				    continue;	/* counted as inverse */
				}
				while ((*tlp-- = *slp--) != 0);
			    }
			    *tlp = gp;	/* swap in real input */
			}
			if (df) printf("	%c%s,",
			    ((i >> 1) & 0x1) ^ inversion ? '~' : ' ', gp->gt_ids);
			op->gt_val++;		/* count input */
			if (gp->gt_fni == GATE) {
			    gp->gt_mark++;	/* logic output at gp */
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
			    goto failed;	/* to avoid memory access error */
			}
		    }
		} while ((i = -i) != 1);
	    } else {
		/********************************************************************
		 *  remaining types not touched in part A
		 *******************************************************************/
		if (df) printf(" %s\t\t\t\t\tlink count = %d\n", op->gt_ids, link_count);
		goto partB;			/* rest not touched in in part A */
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
		link_count++;			/* 1 terminator for ARITH */
		if (op->gt_fni == GATE) {
		    link_count++;		/* 2 terminators for GATE */
		}
	    } else
	    /********************************************************************
	     *  action nodes
	     *******************************************************************/
	    if (op->gt_fni >= MIN_ACT && op->gt_fni < MAX_ACT) {
		op->gt_mark++;			/* count self */
		if ((lp = op->gt_list) == 0 || (gp = *lp++) == 0) {
		    inError(__LINE__, op, 0, "PASS 1: no slave node or funct for action");
		} else
		if (op->gt_fni != F_SW && op->gt_fni != F_CF && op->gt_fni != F_CE) {
		    gp->gt_val++;		/* mark slave node */
		    if (op->gt_fni == D_SH) {
			if (gp->gt_rlist) {
			    inError(__LINE__, op, gp, "PASS 1: backlink for D_SH slave already installed");
			} else {
			    gp->gt_rlist = (Gate**)op;	/* back pointer to delay master */
			}
		    }
		}
		if ((gp = *lp++) == 0 || (gp->gt_fni != CLCKL && gp->gt_fni != TIMRL)) {
		    inError(__LINE__, op, 0, "PASS 1: action has no clock or timer");
		} else {
		    while (gp->gt_ini == -ALIAS) {
			gp = (Gate*)gp->gt_rlist;	/* adjust */
		    }
		    gp->gt_mark++;		/* clock ir timer node */
		    if (gp->gt_fni == TIMRL) {
			if ((gp = *lp++) == 0) {
			    inError(__LINE__, op, 0, "PASS 1: timed action has no delay");
			} else {
			    while (gp->gt_ini == -ALIAS) {
				gp = (Gate*)gp->gt_rlist;	/* adjust */
			    }
			    gp->gt_old++;		/* timer delay */
			}
		    }
		}
	    }
	}
    }
    if (df) printf(" link count = %d\n", link_count);
    if (errCount) {
	exit(1);		/* pass 1 failed */
    }

/********************************************************************
 *
 *	INTERLUDE 1
 *
 *	Allocate space for the output activity lists for ARITH and
 *	GATE nodes.
 *
 *	Allocate space for the symbol table array.
 *
 *******************************************************************/

    ifp = fp = (Gate **)calloc(link_count, sizeof(Gate *));	/* links */
    sTable = sTend = (Gate **)calloc(val, sizeof(Gate *));	/* node* */

/********************************************************************
 *
 *	PASS 2
 *
 *	Enter iClock and each node in i_list into symbol table sTable.
 *	At the end of this pass sTend will hold the end of the table.
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
    /* iClock has no input, needs no output - just report for completeness */
    if (df) printf(" %-8s %3d %3d\n", iClock.gt_ids, iClock.gt_val,
	iClock.gt_mark);
    *sTend++ = &iClock;			/* enter iClock into sTable */

    for (oppp = i_list; (opp = *oppp++) != 0; ) {
	for (op = *opp; op != 0; op = op->gt_next) {
	    *sTend++ = op;		/* enter node into sTable */
	    if (op->gt_ini != -ALIAS) {
		if (df) {
		    printf(" %-8s %3d %3d", op->gt_ids, op->gt_val, op->gt_mark);
		    if (op->gt_old) {
#if INT_MAX == 32767 && defined (LONG16)
			printf(" %3ld\n", op->gt_old);	/* delay references */
#else
			printf(" %3d\n", op->gt_old);	/* delay references */
#endif
		    } else {
			printf("\n");
		    }
		}
		if (op->gt_val == 0 &&
		    op->gt_ini != -ARNC &&
		    op->gt_ini != -LOGC &&
		    op->gt_ini != -INPB &&
		    op->gt_ini != -INPW &&
		    op->gt_ini != -INPX) {
		    fprintf(stderr, "WARNING '%s' has no input\n", op->gt_ids);
		}
		if (op->gt_mark == 0 &&
		    op->gt_old == 0 &&
		    op->gt_fni != TRAB &&
		    op->gt_fni != OUTW &&
		    op->gt_fni != OUTX) {
		    fprintf(stderr, "WARNING '%s' has no output\n", op->gt_ids);
		}
		if (op->gt_fni == ARITH) {
		    op->gt_old = 0;	/* clear for run time after ARITH was delay */
		    fp += op->gt_mark;
		    *fp = 0;		/* last output terminator */
		    op->gt_list = fp++;
		} else
		if (op->gt_fni == GATE) {
		    fp += op->gt_mark + 1;
		    *fp = 0;		/* last output terminator */
		    op->gt_list = fp++;
		}
		if (op->gt_fni != OUTW && op->gt_fni != OUTX) {
		    op->gt_mark = 0;	/* must be cleared for run-time */
		}
	    } else
	    if (df) {
		int inverse = 0;		/* print ALIAS in symbol table */
		printf(" %s@	", op->gt_ids);
		gp = op;
		while (gp->gt_ini == -ALIAS) {
		    if (gp->gt_fni == GATE) {
			inverse ^= gp->gt_mark;	/* holds ALIAS inversion flag */
		    }
		    gp = (Gate*)gp->gt_rlist;	/* resolve ALIAS */
		}
		printf("%s%s\n", inverse ? "~" : " ", gp->gt_ids);
	    }
	}
    }

    if (val != (sTend - sTable)) {
	fprintf(stderr,
	    "\nERROR: %s: line %d: Gate allocation %d vs Symbol Table size %d\n",
	    __FILE__, __LINE__, val, sTend - sTable);
	exit(2);
    }

    if ((val = fp - ifp) != link_count) {
	fprintf(stderr,
	    "\nERROR: %s: line %d: link error %d vs link_count %d\n",
	    __FILE__, __LINE__, val, link_count);
	exit(2);
    }

/********************************************************************
 *
 *	INTERLUDE 2
 *
 *	Sort the symbol table in order of gt_ids.
 *
 *******************************************************************/

    qsort(sTable, sTend - sTable, sizeof(Gate*), (fptr)cmp_gt_ids);

/********************************************************************
 *
 *	PASS 3
 *
 *	Transfer LOGIC inverted inputs, storing them via gp->gt_list,
 *	which is pre-decremented. (same in PASS 4 and 5)
 *
 *******************************************************************/

    if (df) { printf("PASS 3\n"); fflush(stdout); }
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	if (op->gt_ini > 0) {
	    lp = op->gt_rlist;			/* reverse or input list */
	    while (*lp++ != 0);			/* skip normal inputs */
	    while ((gp = *lp++) != 0) {
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
	if (op->gt_ini != -ALIAS && op->gt_fni == GATE) {
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
	if (op->gt_ini == -ARN || op->gt_ini > 0) {
	    lp = op->gt_rlist + (op->gt_ini == -ARN);
	    while ((gp = *lp++) != 0) {
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
			full_type[-op->gt_ini], full_ftype[op->gt_fni]);
		} else {
		    printf(" %-8s%6d%7s:", op->gt_ids,
			op->gt_ini, full_ftype[op->gt_fni]);
		}
	    }
	    if (op->gt_ini == -ARN) {
#ifdef HEXADDRESS
		if (df) printf("	%p()", gp);	/* cexe_n */
#else
		if (df) printf("	0x0()");	/* dummy */
#endif
	    } else
	    /********************************************************************
	     *  arithmetic or logical input linkage to physical I/O
	     *******************************************************************/
	    if (op->gt_ini == -INPW) {
		char *	aName = "??_";
		if ((i = sscanf(op->gt_ids, "%1[IT]%1[XBWL]%5d%7s",
			iqt, xbwl, &byte, tail)) == 3 &&
		    byte < IXD) {
		    switch (iqt[0]) {
		    case 'I':
			switch (xbwl[0]) {
			case 'X':
			    IX_[byte] = op;		/* forward input link */
			    aName = "IX_";
			    break;
			case 'B':
			    IB_[byte] = op;		/* forward input link */
			    aName = "IB_";
			    break;
			case 'W':
			    IW_[byte] = op;		/* forward input link */
			    aName = "IW_";
			    break;
#if INT_MAX != 32767 || defined (LONG16)
			case 'L':
			    IL_[byte] = op;		/* forward input link */
			    aName = "IL_";
			    break;
#endif
			default:
			    goto inErr;
			}
			break;
		    case 'T':
			TX_[byte] = op;			/* forward input link */
			aName = "TX_";
			if (byte >= TXD) goto inErr;
			break;
		    default:
			goto inErr;
		    }
		    if (df) printf("	%s[%d]", aName, byte);
		} else {
		  inErr:
		    inError(__LINE__, op, 0, "PASS 6: no valid input word definition");
		}
	    }
	    if (op->gt_fni == UDFA) {
		inError(__LINE__, op, 0, "PASS 6: has undefined output type UDFA");
	    } else
	    if (op->gt_fni < MIN_ACT) {
		if (df) {
		    lp = op->gt_list;			/* ARITH or GATE */
		    while ((gp = *lp++) != 0) {
			printf("	%s,", gp->gt_ids);
		    }
		    if (op->gt_fni == GATE) {
			while ((gp = *lp++) != 0) {
			    printf("	~%s,", gp->gt_ids);
			}
		    }
		}
	    } else
	    if (op->gt_fni < MAX_ACT) {
		if ((lp = op->gt_list) == 0 ||		/* RI_BIT to TIMR */
		    (gp = *lp++) == 0) {
		    inError(__LINE__, op, 0, "PASS 6: no slave for master action RI_BIT to TIMR");
		} else
		if (op->gt_fni != F_SW && op->gt_fni != F_CF && op->gt_fni != F_CE) {
		    if (df) printf("	%s,", gp->gt_ids);
		} else {
#ifdef HEXADDRESS
		    if (df) printf("	%p(),", gp);	/* cexe_n */
#else
		    if (df) printf("	0x0(),");	/* dummy */
#endif
		}
		if ((gp = *lp++) == 0) {
		    inError(__LINE__, op, 0, "PASS 6: no clock or timer for master action");
		} else
		if (gp->gt_fni != CLCKL && gp->gt_fni != TIMRL) {
		    inError(__LINE__, op, gp, "PASS 6: strange clock or timer");
		} else {
		    if (gp->gt_ini == -ALIAS) {		/* resolve clock/timer alias */
			while (gp->gt_ini == -ALIAS) {
			    if (df) printf("	%s@", gp->gt_ids);
			    gp = (Gate*)gp->gt_rlist;	/* adjust */
			}
			tlp = lp - 1;
			*tlp = gp;			/* swap in real clock or timer */
		    }
		    if ((gp->gt_fni != CLCKL || gp->gt_ini != -CLK) &&
			(gp->gt_fni != TIMRL || gp->gt_ini != -TIM)) {
			inError(__LINE__, op, gp, "PASS 6: clock or timer master does not match slave");
		    }
		    if (df) printf("	%c%s,", os[-gp->gt_ini], gp->gt_ids);
		    if (gp->gt_fni == TIMRL) {
			if ((gp = *lp++) == 0) {
			    inError(__LINE__, op, 0, "PASS 6: timed action has no delay");
			} else {
			    if (gp->gt_ini == -ALIAS) {	/* resolve arithmetic alias */
				while (gp->gt_ini == -ALIAS) {
				    if (df) printf("	%s@", gp->gt_ids);
				    gp = (Gate*)gp->gt_rlist;	/* adjust */
				}
				tlp = lp - 1;
				*tlp = gp;		/* swap in real input */
			    }
			    if (df) printf("	<%s,", gp->gt_ids);
			}
		    }
		}
	    } else
	    /********************************************************************
	     *  arithmetic or logical output linkage to physical I/O
	     *  QX_[slot] receives output value in outMw()
	     *******************************************************************/
	    if (op->gt_fni == OUTW) {
		if ((i = sscanf(op->gt_ids, "Q%1[XBWL]%5d%7s", xbwl, &byte, tail)) >= 2 &&
		    byte < IXD) {
		    op->gt_list = (Gate **) byte;	/* slot */
		    QT_[byte] = xbwl[0];
		    switch (xbwl[0]) {
		    case 'X':
			if (i > 2) goto outErr;	/* no tail _0 allowed for QXn */
			op->gt_mark = X_WIDTH;
			break;
		    case 'B':
			op->gt_mark = B_WIDTH;
			break;
		    case 'W':
			op->gt_mark = W_WIDTH;
			break;
#if INT_MAX != 32767 || defined (LONG16)
		    case 'L':
			op->gt_mark = L_WIDTH;
			break;
#endif
		    default:
			goto outErr;
		    }
		    if (df) printf("	QX_[%d]	0x%02x", (int)op->gt_list, op->gt_mark);
		} else {
		  outErr:
		    inError(__LINE__, op, 0, "PASS 6: no valid output word definition");
		}
	    } else
	    if (op->gt_fni < CLCKL) {
		if (df) {
		    if (op->gt_fni == OUTX) {
			gp = (Gate*)(op->gt_list);	/* OUTX */
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
#else
		    if (df) printf("	(%d)", op->gt_old);
#endif
		}
	    } else {
		inError(__LINE__, op, 0, "PASS 6: unknown output type (ftype)");
	    }
	    if (df) printf("\n");
	} else
	if (df) {
	    int inverse = 0;		/* print ALIAS in symbol table */
	    gp = op;
	    printf(" %-8s%6s%7s:", op->gt_ids,
		full_type[-op->gt_ini], full_ftype[op->gt_fni]);
	    while (gp->gt_ini == -ALIAS) {
		if (gp->gt_fni == GATE) {
		    inverse ^= gp->gt_mark;	/* holds ALIAS inversion flag */
		}
		gp = (Gate*)gp->gt_rlist;	/* resolve ALIAS */
	    }
	    printf("	%s%s\n", inverse ? "~" : "", gp->gt_ids);
	}
    }
    if (errCount) {
	exit(6);		/* pass 6 failed */
    }
    icc(&iClock, &errCount);	/* try it like this */
    return 0;
} /* main */
#endif

static const char load_c[] =
"@(#)$Id: load.c,v 1.42 2004/03/07 17:32:41 jw Exp $";
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

extern const char	SC_ID[];

typedef int (*fptr)(const void*, const void*);

Gate		iClock = { -1, -CLK, CLCKL, 0, "iClock" };	/* active */
Gate **		sTable;			/* pointer to dynamic array */
Gate **		sTend;			/* end of dynamic array */
unsigned long	sTstrLen;		/* length of symbol strings */

unsigned	errCount;
const char *	progname;		/* name of this executable */
short		debug = 0;
int		micro = 0;
unsigned short	xflag;
unsigned short	osc_max = MARKMAX;

FILE *		outFP;			/* listing file pointer */
FILE *		errFP;			/* error file pointer */

static const char *	usage =
"USAGE: %s [-"
#if YYDEBUG
"t"
#endif
"xh]"
#ifdef TCP
" [-m[m]] [-s <server>] [-p <port>] [-u <unitID>]\n      "
#endif
" [-n<count>] [-d<debug>]\n"
#ifdef TCP
"        -s host ID of server      (default '%s')\n"
"        -p service port of server (default '%s')\n"
"        -u unit ID of this client (default '%s')\n"
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
"%s\n";

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

Gate *		IX_[IXD*8];		/* pointers to bit Input Gates */
Gate *		IB_[IXD];		/* pointers to byte Input Gates */
Gate *		IW_[IXD];		/* pointers to word Input Gates */
#if INT_MAX != 32767 || defined (LONG16)
Gate *		IL_[IXD];		/* pointers to long Input Gates */
#endif
Gate *		TX_[TXD*8];		/* pointers to bit System Gates */
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
    unsigned	inversion;
    unsigned	val;
    unsigned	link_count = 0;
    Gate **	fp;
    Gate **	ifp;
    int		i;
    unsigned	df = 0;

    /* Process the arguments */
    if ((progname = strrchr(*argv, '/')) == NULL) {
	progname = *argv;
    } else {
	progname++;		/*  path has been stripped */
    }
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
		case 'u':
		    if (! *++*argv) { --argc, ++argv; }
		    iccNM = *argv;
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
		    osc_max = atoi(*argv);
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
		    hostNM, portNM, iccNM,
#endif
		    MARKMAX, SC_ID);
		    exit(1);
		}
	    } while (*++*argv);
	    break2: ;
	} else {
	    goto error;
	}
    }
    debug &= 03743;			/* allow only cases specified */

/********************************************************************
 *
 *	PASS 0
 *
 *	Clear all gt_mark and gt_val entries. (not strictly necessary)
 *	except gt_mark for OUTW and OUTX which holds output config data
 *	and ALIAS nodes which hold inversion flag in gt_mark.
 *	INPW and INPX nodes have gt_val initialised to -1.
 *
 *	Transfer timer preset value in gt_mark to gt_old for run time
 *
 *******************************************************************/

    if (df) { printf("PASS 0\n"); fflush(stdout); }
    for (oppp = i_list; (opp = *oppp++) != 0; ) {
	for (op = *opp; op != 0; op = op->gt_next) {
	    op->gt_val = (op->gt_ini == -NCONST	||
			  op->gt_ini == -INPW	||
			  op->gt_ini == -INPX	) ? -1 : 0;
	    if (op->gt_ini != -ALIAS &&
		op->gt_fni != OUTW && op->gt_fni != OUTX) {
		if (op->gt_fni == TIMRL) {
		    op->gt_old = op->gt_mark;
		} else {
		    op->gt_old = 0;
		}
		op->gt_mark = 0;
	    }
	}
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

    if (df) { printf("PASS 1\n"); fflush(stdout); }
    val = 1;					/* count iClock */
    for (oppp = i_list; (opp = *oppp++) != 0; ) {
	for (op = *opp; op != 0; op = op->gt_next) {
	    val++;				/* count node */
	    if (op->gt_ini == -ARN) {
		if ((lp = op->gt_rlist) == 0) {
		    inError(__LINE__, op, 0, "arithmetic node with no forward list");
		    goto ag1;
		}
		if (df) printf(" %-8s%3d:", op->gt_ids, op->gt_ini);
		lp++;				/* skip function vector */
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
		    } else {
			inError(__LINE__, op, gp, "arithmetic node points back to non ARITH");
		    }
		}
		if (df) printf("\n");
	    } else if (op->gt_ini > 0) {
		if ((lp = op->gt_rlist) == 0) {
		    inError(__LINE__, op, 0, "logic node with no forward list");
		    goto ag1;
		}
		if (df) printf(" %-8s%3d:", op->gt_ids, op->gt_ini);
		i = 1;			/* LOGIC nodes AND, OR or LATCH */
		/* for LOGIC scan 2 lists with i = 1 and -1 */
		do {
		    while ((gp = *lp++) != 0) {
			inversion = 0;		/* resolve logical ALIAS */
			if (gp->gt_ini == -ALIAS || gp->gt_fni == OUTX) {
			    while (gp->gt_ini == -ALIAS || gp->gt_fni == OUTX) {
				if (gp->gt_ini == -ALIAS) {
				    if (df) printf("	%c%s@",		/* @ */
					inversion ? '~' : ' ', gp->gt_ids);
				    inversion ^= gp->gt_mark;
				    gp = (Gate*)gp->gt_rlist;
				} else {
				    unsigned iv = 0;			/* @ */
				    if ((tlp = gp->gt_rlist) == 0) {
					inError(__LINE__, op, gp, "ALIAS without link list");
				    } else if ((tgp = *tlp++) == 0) {
					iv = 1;				/* @ */
					inversion ^= 1;
					if ((tgp = *tlp++) == 0) {
					    inError(__LINE__, op, gp, "ALIAS without link node");
					}
				    }
				    if (df) printf("	%c%s>,",	/* @ */
					iv ? '~' : ' ', gp->gt_ids);	/* @ */
				    gp = tgp;
				}
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
				inError(__LINE__, op, gp, "logic node points to non GATE");
			    } else {
				inError(__LINE__, op, 0, "logic node points to non Gate struct");
			    }
			    fprintf(stderr,
				"ERROR %s: line %d: ftype = %d type = %d ???\n",
				__FILE__, __LINE__, gp->gt_fni, gp->gt_ini);
			    goto failed;	/* to avoid memory access error */
			}
		    }
		} while ((i = -i) != 1);
	    failed:
		if (df) printf("\n");
	    }
	ag1:
	    if (op->gt_ini != -ALIAS &&
		(op->gt_fni == ARITH || op->gt_fni == GATE)) {
		if (op->gt_list != 0) {
		    inError(__LINE__, op, 0, "no forward list for ARITH or GATE");
		}
		link_count++;			/* 1 terminator for ARITH */
		if (op->gt_fni == GATE) {
		    link_count++;		/* 2 terminators for GATE */
		}
	    } else if (op->gt_fni >= MIN_ACT && op->gt_fni < MAX_ACT) {
		op->gt_mark++;			/* count self */
		if ((lp = op->gt_list) == 0 || (gp = *lp++) == 0) {
		    inError(__LINE__, op, 0, "no slave node or funct for action");
		} else if (op->gt_fni != F_SW && op->gt_fni != F_CF && op->gt_fni != F_CE) {
		    gp->gt_val++;		/* mark slave node */
		    if (op->gt_fni == D_SH) {
			if (gp->gt_rlist) {
			    inError(__LINE__, op, gp, "backlink for D_SH slave already installed");
			} else {
			    (Gate*)gp->gt_rlist = op;	/* back pointer to delay master */
			}
		    }
		}
		if ((gp = *lp++) == 0 || (gp->gt_fni != CLCKL && gp->gt_fni != TIMRL)) {
		    inError(__LINE__, op, 0, "action has no clock or timer");
		} else {
		    while (gp->gt_ini == -ALIAS) {
			gp = (Gate*)gp->gt_rlist;	/* adjust */
		    }
		    gp->gt_mark++;		/* clock ir timer node */
		    if (gp->gt_fni == TIMRL) {
			if ((gp = *lp++) == 0) {
			    inError(__LINE__, op, 0, "timed action has no delay");
			} else {
			    while (gp->gt_ini == -ALIAS) {
				gp = (Gate*)gp->gt_rlist;	/* adjust */
			    }
			    gp->gt_old++;		/* timer delay */
			}
		    }
		}
	    } else if (op->gt_fni == OUTW) {
		int	slot;
		int	width;
		if ((slot = (int)op->gt_list) < IXD && (width = op->gt_mark) != 0) {
		    QT_[slot] = width == B_WIDTH ? 'B' :
				width == W_WIDTH ? 'W' :
#if INT_MAX != 32767 || defined (LONG16)
				width == L_WIDTH ? 'L' :
#endif
						    0;
		} else {
		    inError(__LINE__, op, 0, "no valid output word definition");
		}
	    } else if (op->gt_fni == OUTX) {
		int	slot;
		if ((slot = (int)op->gt_list) < IXD && op->gt_mark) {
		    QT_[slot] = 'X';
		} else {
		    inError(__LINE__, op, 0, "no valid output bit definition");
		}
	    }
	}
    }
    if (df) printf(" link count = %d\n", link_count);
    if (errCount) {
	exit(2);		/* pass 1 failed */
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
 *	Maintain a long value sTstrLen which reports how much space
 *	must be allocated to hold a copy of all the ID strings.
 *	The length of a pointer array for such a symbol table is
 *	sTend - sTable.
 *
 *******************************************************************/

    if (df) { printf("PASS 2 - symbol table: name inputs outputs delay-references\n"); fflush(stdout); }
    /* iClock has no input, needs no output - just report for completeness */
    if (df) printf(" %-8s %3d %3d\n", iClock.gt_ids, iClock.gt_val,
	iClock.gt_mark);
    *sTend++ = &iClock;			/* enter iClock into sTable */
    sTstrLen = strlen(iClock.gt_ids) + 1;	/* initialise length */

    for (oppp = i_list; (opp = *oppp++) != 0; ) {
	for (op = *opp; op != 0; op = op->gt_next) {
	    *sTend++ = op;		/* enter node into sTable */
	    sTstrLen += strlen(op->gt_ids) + 1;	/* string length */
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
		if (op->gt_val == 0 && op->gt_ini != -LOGC && op->gt_ini != -ARNC) {
		    fprintf(stderr, "WARNING '%s' has no input\n", op->gt_ids);
		}
		if (op->gt_mark == 0 && op->gt_old == 0) {
		    fprintf(stderr, "WARNING '%s' has no output\n", op->gt_ids);
		}
		if (op->gt_fni == ARITH) {
		    op->gt_old = 0;	/* clear for run time */
		    fp += op->gt_mark;
		    *fp = 0;		/* last output terminator */
		    op->gt_list = fp++;
		} else if (op->gt_fni == GATE) {
		    fp += op->gt_mark + 1;
		    *fp = 0;		/* last output terminator */
		    op->gt_list = fp++;
		}
		if (op->gt_fni != OUTW && op->gt_fni != OUTX) {
		    op->gt_mark = 0;	/* must be cleared for run-time */
		}
	    } else if (df) {
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

    if ((val = fp - ifp) != link_count) {
	fprintf(stderr,
	    "\nERROR: %s: line %d: link error %d vs link_count %d\n",
	    __FILE__, __LINE__, val, link_count);
	exit(3);
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
 *	C files or the PPLC compiler is in error (!!)
 *
 *******************************************************************/

    if (df) { printf("PASS 6 - name gt_ini gt_fni:\n"); fflush(stdout); }
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
	    } else if (op->gt_ini == -INPW || op->gt_ini == -INPX) {
		if ((lp = op->gt_rlist) == 0) {
		    inError(__LINE__, op, 0, "no forward input link");
		} else {
		    *lp = op;		/* forward input link */
		}
		if (df) {
		    int		i;
		    char *	aName = "??_";
		    if ((i = lp - IX_) >= 0 && i < IXD*8) {
			aName = "IX_";
		    } else if ((i = lp - IB_) >= 0 && i < IXD) {
			aName = "IB_";
		    } else if ((i = lp - IW_) >= 0 && i < IXD) {
			aName = "IW_";
#if INT_MAX != 32767 || defined (LONG16)
		    } else if ((i = lp - IL_) >= 0 && i < IXD) {
			aName = "IL_";
#endif
		    } else if ((i = lp - TX_) >= 0 && i < TXD*8) {
			aName = "TX_";
		    }
		    printf("	%s[%d]", aName, i);
		}
	    }
	    if (op->gt_fni == UDFA) {
		inError(__LINE__, op, 0, "has undefined output type UDFA");
	    } else if (op->gt_fni < MIN_ACT) {
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
	    } else if (op->gt_fni < MAX_ACT) {
		if ((lp = op->gt_list) == 0 ||		/* RI_BIT to TIMR */
		    (gp = *lp++) == 0) {
		    inError(__LINE__, op, 0, "no slave for master action RI_BIT to TIMR");
		} else if (op->gt_fni != F_SW && op->gt_fni != F_CF && op->gt_fni != F_CE) {
		    if (df) printf("	%s,", gp->gt_ids);
		} else {
#ifdef HEXADDRESS
		    if (df) printf("	%p(),", gp);	/* cexe_n */
#else
		    if (df) printf("	0x0(),");	/* dummy */
#endif
		}
		if ((gp = *lp++) == 0) {
		    inError(__LINE__, op, 0, "no clock or timer for master action");
		} else if (gp->gt_fni != CLCKL && gp->gt_fni != TIMRL) {
		    inError(__LINE__, op, gp, "strange clock or timer");
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
			inError(__LINE__, op, gp, "clock or timer master does not match slave");
		    }
		    if (df) printf("	%c%s,", os[-gp->gt_ini], gp->gt_ids);
		    if (gp->gt_fni == TIMRL) {
			if ((gp = *lp++) == 0) {
			    inError(__LINE__, op, 0, "timed action has no delay");
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
	    } else if (op->gt_fni < CLCKL) {
		/* consistency of OUTW and OUTX checked in Pass 1 */
		if (df) printf("	QX_[%d]	0x%02x", (int)op->gt_list, op->gt_mark);
	    } else if (op->gt_fni <= TIMRL) {
		if (op->gt_list != 0) {
		    inError(__LINE__, op, 0, "clock or timer with initial forward list");
		}
		if (op->gt_fni == TIMRL && op->gt_old > 0) {
#if INT_MAX == 32767 && defined (LONG16)
		    if (df) printf("	(%ld)", op->gt_old);
#else
		    if (df) printf("	(%d)", op->gt_old);
#endif
		}
	    } else {
		inError(__LINE__, op, 0, "unknown output type (ftype)");
	    }
	    if (df) printf("\n");
	} else if (df) {
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
	exit(5);		/* pass 6 failed */
    }
    icc(&iClock, &errCount);	/* try it like this */
    return 0;
} /* main */
#endif

static const char load_c[] =
"@(#)$Id: load.c,v 1.5 2000/05/31 10:45:07 jw Exp $";
/********************************************************************
 *
 *	load.c
 *
 *	This module prepares the data structures for the run time
 *	system from the loaded data and calls the pplc execution.
 *
 *	"load.c	1.01	95/02/15"
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"pplc.h"

#define STS	1

typedef int (*fptr)(const void*, const void*);

Gate		iClock = { -1, -CLK, CLCKL, 0, "iClock" };	/* active */
Gate **		sTable;			/* pointer to dynamic array */
Gate **		sTend;			/* end of dynamic array */
unsigned long	sTstrLen;		/* length of symbol strings */

unsigned	errCount;
char *		progname;
short		debug;
unsigned short	aaflag;
unsigned short	osc_max = MARKMAX;

char *		full_type[] = { FULL_TYPE };
char *		full_ftype[] = { FULL_FTYPE };
uchar		types[] = { TYPES };
uchar		ftypes[] = { FTYPES };
char		os[] = OPS;
char		fos[] = FOPS;

FILE *		outFP;			/* listing file pointer */
FILE *		errFP;			/* error file pointer */

static char *	usage = "USAGE:\n\
%s [-d<debug>] [-n<count>] [-ah]\n\
	-d <debug>2000	display scan_cnt and link_cnt\n\
		  +400	exit after initialisation\n\
		  +200	display loop info (+old style logic)\n\
		  +100	initialisation and run time info\n\
		   +40	net statistics\n\
	-n <count>	maxinum loop count (default is %d, limit 15)\n\
	-a		start run time arithmetic info in decimal\n\
			can be changed at run time with d or x\n\
	-h		this help text\n\
";

/********************************************************************
 *
 *	Compare gt_ids in two Gates support of qsort()
 *
 *******************************************************************/

int cmp_gt_ids( const Gate **a, const Gate **b)
{
    return( strcmp((*a)->gt_ids, (*b)->gt_ids) );
}

/********************************************************************
 *
 *	Error message routine for errors in loading application
 *
 *******************************************************************/

void
inError(Gate * op, Gate * gp)
{
    fprintf(stderr,
	"	ERROR op = %s gp = %s\n", op->gt_ids, gp ? gp->gt_ids : "0");
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
    char * *	argv)
{
    register Gate *	op;
    register Gate *	gp;
    Gate ***	oppp;
    Gate **	opp;
    Gate **	lp;
    Gate **	slp;
    Gate **	tlp;
    Gate *	tgp;
    unsigned	inversion;
    unsigned	val;
    unsigned	link_count = 0;
    Gate **	fp;
    Gate **	ifp;
    int		i;
    unsigned	df = 0;

    progname = *argv;		/* Process the arguments */
    outFP = stdout;		/* listing file pointer */
    errFP = stderr;		/* error file pointer */
    while (--argc > 0) {
	if (**++argv == '-') {
	    ++*argv;
	    do {
		switch (**argv) {
		case 'd':
		    if (! *++*argv) { --argc, ++argv; }
		    sscanf(*argv, "%o", &df);
		    debug = df;		/* short */
		    df &= 040;	/* net statistics */
		    goto break2;
		case 'n':
		    if (! *++*argv) { --argc, ++argv; }
		    osc_max = atoi(*argv);
		    if (osc_max > 15) goto error;
		    goto break2;
		case 'a':
		    aaflag = 1;		/* start with decimal output */
		    break;
		default:
		    fprintf(stderr,
			"%s: unknown flag '%c'\n", progname, **argv);
		case 'h':
		case '?':
		error:
		    fprintf(stderr, usage, progname, MARKMAX);
		    exit(1);
		}
	    } while (*++*argv);
	    break2: ;
	} else {
	    goto error;
	}
    }

/********************************************************************
 *
 *	PASS 0
 *
 *	Clear all gt_mark and gt_val entries. (not strictly necessary)
 *	except gt_mark for OUTW and OUTX which holds output config data
 *	and ALIAS nodes which hold inversion flag in gt_mark.
 *	INPW and INPX nodes have gt_val initialised to -1.
 *
 *******************************************************************/

    if (df) printf("PASS 0\n");
    for (oppp = i_list; (opp = *oppp++) != 0; ) {
	for (op = *opp; op != 0; op = op->gt_next) {
	    op->gt_val = op->gt_ini == -INPW || op->gt_ini == -INPX ? -1 : 0;
	    if (op->gt_ini != -ALIAS &&
		op->gt_fni != OUTW && op->gt_fni != OUTX) {
		op->gt_mark = 0;
	    }
	}
    }

/********************************************************************
 *
 *	All the activity lists for action nodes are already correctly
 *	compiled and do not need to be altered.
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
 *	in op->gt_val for a consistency check.
 *
 *	For each ARITH and GATE node op, which is not an ALIAS, count
 *	1 extra link for ARITH nodes and 2 extra links for GATE nodes.
 *	These are space for the activity list terminators.
 *
 *	Resolve inputs from GATE ALIAS nodes and OUTW nodes, which
 *	may occurr because of multiple modules linked together.
 *	After this pass the input lists contain no aliases.
 *
 *******************************************************************/

    if (df) printf("PASS 1\n");
    val = 1;					/* count iClock */
    for (oppp = i_list; (opp = *oppp++) != 0; ) {
	for (op = *opp; op != 0; op = op->gt_next) {
	    val++;				/* count node */
	    if (op->gt_ini == -ARN) {
		if ((lp = op->gt_rlist) == 0) { inError(op, 0); goto ag1; }
		if (df) printf("%-8s%3d:", op->gt_ids, op->gt_ini);
		lp++;			/* skip function vector */
		while ((gp = *lp++) != 0) { /* for ARN scan 1 list */
		    if (gp->gt_ini == -ALIAS) {	/* should not occurr */
			while (gp->gt_ini == -ALIAS) {
			    gp = (Gate*)gp->gt_rlist;	/* adjust */
			}
			tlp = lp - 1;
			*tlp = gp;	/* swap in real input */
		    }
		    if (df) printf("	%s,", gp->gt_ids);
		    op->gt_val++;	/* count input */
		    if (gp->gt_fni == ARITH) {
			gp->gt_mark++;	/* arithmetic output at gp */
			link_count++;
		    } else {
			inError(op, gp);
		    }
		}
		if (df) printf("\n");
	    } else if (op->gt_ini > 0) {
		if ((lp = op->gt_rlist) == 0) { inError(op, 0); goto ag1; }
		if (df) printf("%-8s%3d:", op->gt_ids, op->gt_ini);
		i = 1;			/* LOGIC nodes AND, OR or LATCH */
		/* for LOGIC scan 2 lists with i = 1 and -1 */
		do {
		    while ((gp = *lp++) != 0) {
			inversion = 0;
			if (gp->gt_ini == -ALIAS ||
			    gp->gt_fni == OUTX) {
			    while (gp->gt_ini == -ALIAS ||
				gp->gt_fni == OUTX) {
				if (gp->gt_ini == -ALIAS) {
				    inversion ^= gp->gt_mark;
				    gp = (Gate*)gp->gt_rlist;
				} else {
				    if ((tlp = gp->gt_rlist) == 0) {
					inError(op, gp);
				    } else if ((tgp = *tlp++) == 0) {
					inversion ^= 1;
					if ((tgp = *tlp++) == 0) {
					    inError(op, gp);
					}
				    }
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
			if (df) printf("	%s%s,",
			    (i >> 1) & 1 ^ inversion ? "~" : "",
			    gp->gt_ids);
			op->gt_val++;		/* count input */
			if (gp->gt_fni == GATE) {
			    gp->gt_mark++;	/* logic output at gp */
			    link_count++;
			} else {
			    inError(op, gp);
			}
		    }
		} while ((i = -i) != 1);
		if (df) printf("\n");
	    }
	ag1:
	    if (op->gt_ini != -ALIAS &&
		(op->gt_fni == ARITH || op->gt_fni == GATE)) {
		if (op->gt_list != 0) {
		    inError(op, 0);
		}
		link_count++;			/* 1 terminator for ARITH */
		if (op->gt_fni == GATE) {
		    link_count++;		/* 2 terminators for GATE */
		}
	    } else if (op->gt_fni >= MIN_ACT && op->gt_fni < MAX_ACT) {
		op->gt_mark++;			/* count self */
		if ((lp = op->gt_list) == 0 || (gp = *lp++) == 0) {
		    inError(op, 0);		/* no slave node or funct */
		} else if (op->gt_fni != F_CF) {
		    gp->gt_val++;		/* slave node */
		}
		if ((gp = *lp) == 0) {
		    inError(op, 0);		/* no clock reference */
		} else {
		    gp->gt_mark++;		/* clock node */
		}
	    } else if (op->gt_fni == OUTW || op->gt_fni == OUTX) {
		if ((lp = op->gt_list) == 0) {
		    inError(op, 0);		/* no output pointer */
		}
	    }
	}
    }
    if (df) printf("link count = %d\n", link_count);
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
 *	Maintain a long value sTstrLen which reports how much space
 *	must be allocated to hold a copy of all the ID strings.
 *	The length of a pointer array for such a symbol table is
 *	sTend - sTable.
 *
 *******************************************************************/

    if (df) printf("PASS 2\n");
    /* iClock has no input, needs no output - just report for completeness */
    if (df) printf("%-8s %3d %3d\n", iClock.gt_ids, iClock.gt_val,
	iClock.gt_mark);
    *sTend++ = &iClock;			/* enter iClock into sTable */
    sTstrLen = strlen(iClock.gt_ids) + 1;	/* initialise length */

    for (oppp = i_list; (opp = *oppp++) != 0; ) {
	for (op = *opp; op != 0; op = op->gt_next) {
	    *sTend++ = op;		/* enter node into sTable */
	    sTstrLen += strlen(op->gt_ids) + 1;	/* string length */
	    if (op->gt_ini != -ALIAS) {
		if (df) printf("%-8s %3d %3d\n",
		    op->gt_ids, op->gt_val, op->gt_mark);
		if (op->gt_val == 0) {
		    fprintf(stderr, "WARNING '%s' has no input\n", op->gt_ids);
		}
		if (op->gt_mark == 0) {
		    fprintf(stderr, "WARNING '%s' has no output\n", op->gt_ids);
		}
		if (op->gt_fni == ARITH || op->gt_fni == GATE) {
		    fp += op->gt_mark + (op->gt_fni == GATE);
		    *fp = 0;		/* last output terminator */
		    op->gt_list = fp++;
		}
		if (op->gt_fni != OUTW && op->gt_fni != OUTX) {
		    op->gt_mark = 0;	/* must be cleared for run-time */
		}
	    }
	}
    }

    if ((val = fp - ifp) != link_count) {
	fprintf(stderr,
	    "\n%s: line %d: link error %d vs link_count %d\n",
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

    if (df) printf("PASS 3\n");
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

    if (df) printf("PASS 4\n");
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

    if (df) printf("PASS 5\n");
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

    if (df) printf("PASS 6\n");
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	if (op->gt_ini != -ALIAS) {
	    if (df) printf("%-8s%3d%3d:",
		op->gt_ids, op->gt_ini, op->gt_fni);
	    if (op->gt_ini == -ARN) {
		if ((gp = *op->gt_rlist) == 0) inError(op, 0);
		if (df) printf("	%p()", gp);
	    } else if (op->gt_ini == -INPW || op->gt_ini == -INPX) {
		if ((lp = op->gt_rlist) == 0) {
		    inError(op, 0);
		} else {
		    *lp = op;		/* forward input link */
		}
		if (df) printf("	%p[]", lp);
	    }
	    if (op->gt_fni == UDFA) {
		inError(op, 0);			/* UDFA */
	    } else if (op->gt_fni < MIN_ACT) {
		lp = op->gt_list;		/* ARITH or GATE */
		while ((gp = *lp++) != 0) {
		    if (df) printf("	%s,", gp->gt_ids);
		}
		if (op->gt_fni == GATE) {
		    while ((gp = *lp++) != 0) {
			if (df) printf("	~%s,", gp->gt_ids);
		    }
		}
	    } else if (op->gt_fni < MAX_ACT) {
		if ((lp = op->gt_list) == 0 || (gp = *lp++) == 0) {
		    inError(op, 0);		/* action D_SH to D_FF */
		} else if (op->gt_fni == F_CF) {
		    if (df) printf("	%p(),", gp);
		} else {
		    if (df) printf("	%s,", gp->gt_ids);
		}
		if ((gp = *lp++) == 0) {
		    inError(op, 0);
		} else {
		    if (df) printf("	%s,", gp->gt_ids);
		    if (gp->gt_fni == TIMRL) {
			if (df) printf("	%p(),", *lp);
		    }
		}
	    } else if (op->gt_fni < CLCKL) {
		if ((lp = op->gt_list) == 0) {
		    inError(op, 0);		/* OUTW or OUTX */
		} else {
		    if (df) printf("	%p[]	0x%02x", lp, op->gt_mark);
		}
	    } else if (op->gt_fni <= TIMRL) {
		if ((lp = op->gt_list) != 0) {
		    inError(op, 0);		/* CLCKL or TIMRL */
		}
	    } else {
		inError(op, 0);			/* unknown ftype */
	    }
	    if (df) printf("\n");
	}
    }
    if (errCount) {
	exit(5);		/* pass 6 failed */
    }
    pplc(&iClock, &errCount);	/* try it like this */
    return 0;
} /* main */

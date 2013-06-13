static const char ict_c[] =
"@(#)$Id: ict.c,v 1.60 2013/05/12 08:10:47 jw Exp $";
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
 *	ict.c
 *	parallel plc - runtime execution with TCP/IP I/O to iCserver
 *
 *	Standalone run-time version for TCP/IP communication.
 *	Alternatively to produce compiled applications
 *	linked with libict.a produced with LOAD option
 *
 *	branched from "icc.c,v 1.6 1999/08/06"
 *
 *******************************************************************/

/* J.E. Wulff	3-Mar-85 */

#ifndef TCP
#error - must be compiled with TCP defined to make a linkable library
#else	/* TCP */
#include	<stdio.h>
#include	<stdlib.h>
#include	<time.h>
#include	<errno.h>
#include	<sys/types.h>
#ifdef	WIN32
#include	<Time.h>
#else	/* WIN32 */
#include	<sys/time.h>
#endif	/* WIN32 */
#include	<signal.h>
#include	<ctype.h>
#include	<assert.h>
#include	"tcpc.h"
#include	"icc.h"

#define D10	10			/* 1/10 second select timeout under Linux */
#define TLIMIT	4			/* TX0.4 is fastest timer */

#ifndef SIGRTMAX
#define SIGRTMAX	32		/* for non-POSIX systems (Win32) */
#endif
#define QUIT_TERMINAL	(SIGRTMAX+3)
#define QUIT_DEBUGGER	(SIGRTMAX+4)
#define QUIT_SERVER	(SIGRTMAX+5)

#define IOCHANNELS	30		/* initial Channels[] size */
#define ENTRYSZ		16		/* entry "SIX0000-000\0" max length 12 */

struct timeval	iC_timeOut = { 0, 50000 };	/* 50 mS select timeout - may be modified in iCbegin() */
static struct timeval	toCnt =   { 0, 50000 };	/* actual timeout counter that select uses */
static struct timeval *	toCntp = NULL;		/* select timeout switched off when NULl pointer */

static void	regAck(Gate ** oStart, Gate ** oEnd);
static void	storeChannel(unsigned short channel, Gate * gp);
static char *
#if	INT_MAX == 32767 && defined (LONG16)
convert2binary(char * binBuf, long value, int bitFlag);
#else	/* INT_MAX == 32767 && defined (LONG16) */
convert2binary(char * binBuf, int value, int bitFlag);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
static iC_Functp *	iC_i_lists[] = { I_LISTS };

/********************************************************************
 *  initialise all lists with their name to aid symbolic debugging
 *  do this here because at least "flist" is needed in load() for
 *  ini output, which runs before iC_icc().
 *******************************************************************/
/* these lists are toggled (initialised dynamically) */
static Gate	alist0 = { 0, 0, 0, 0, "alist0", };
static Gate	alist1 = { 0, 0, 0, 0, "alist1", };
Gate *		iC_a_list;		/* arithmetic output action list */
static Gate	olist0 = { 0, 0, 0, 0, "olist0", };
static Gate	olist1 = { 0, 0, 0, 0, "olist1", };
Gate *		iC_o_list;		/* logic output action list */
/* these lists are not toggled (static initialisation here) */
Gate *		iC_c_list;		/* main clock list "iClock" */
static Gate	flist = { 0, 0, 0, 0, "flist", };
Gate *		iC_f_list = &flist;	/* deferred function action list (init in load) */
static Gate	slist = { 0, 0, 0, 0, "slist", };
Gate *		iC_s_list = &slist;	/* send bit and byte outputs */

short		iC_error_flag;

unsigned	iC_scan_cnt;		/* count scan operations */
unsigned	iC_link_cnt;		/* count link operations */
#if	YYDEBUG && (!defined(_WINDOWS) || defined(LOAD))
unsigned	iC_glit_cnt;		/* count glitches */
unsigned long	glit_nxt;		/* count glitch scan */
#endif	/* YYDEBUG && (!defined(_WINDOWS) || defined(LOAD)) */

static SOCKET	sockFN = 0;		/* TCP/IP socket file number */

static Gate *	timNull[] = { 0, 0, 0, 0, 0, 0, 0, 0, }; /* speeds up tim[] lookup */
static unsigned short	topChannel = 0;
static unsigned short	C_channel = 0;	/* channel for sending messages to Debug */
static Gate **	Channels = NULL;	/* dynamic array to store input gate addresses */
static int	ioChannels = 0;		/* allocated size of Channels[] */
static Gate	D_gate;			/* Gate on channel for receiving messages from Debug */
static int	regOffset = 0;		/* for register send */
static int	liveOffset = 0;		/* for live header */
#ifndef	EFENCE
static char	regBuf[REQUEST];
#else	/* EFENCE */
static char *	regBuf;
#endif	/* EFENCE */

static int	msgOffset = 0;		/* for message send */
#ifndef	EFENCE
static char	msgBuf[REQUEST];
char		iC_outBuf[REQUEST];	/* Buffer in which output is collected in rsff.c */
char		rpyBuf[REPLY];		/* Buffer in which iCserver input is read to */
#else	/* EFENCE */
static char *	msgBuf;
char *		iC_outBuf;
static char *	rpyBuf;
#endif	/* EFENCE */
int		iC_outOffset;
extern const char	iC_ID[];
static FILE *	iC_vcdFP = NULL;
static char *	iC_sav = NULL;
static FILE *	iC_savFP = NULL;
static int	liveIndex = 0;
static long	virtualTime = 0;
static short	iClock_index;
static char *	vcd_ftype[]	= { VCD_FTYPE };
static char *	sav_ftype[]	= { SAV_FTYPE };

/********************************************************************
 *
 *	Procedure iC_icc
 *
 *******************************************************************/

void
iC_icc(Gate ** sTable, Gate ** sTend)
{
    int		i;
    short	pass;
    int		c;
    short	typ;
    int		cnt;
    Gate **	opp;
    char *	cp;
    Gate **	tim = timNull;		/* point to array of 8 null pointers */
    Gate *	gp;
    iC_Functp	init_fa;
    int		tcnt1  = 1;
    int		tcnt10 = 1;
    int		tcnt60 = 1;
    int		retval;
    unsigned short index;
    FILE *	vcdFlag;
    char	binBuf[33];		/* allows 32 bits */

#ifdef	EFENCE
    msgBuf = iC_emalloc(REQUEST);
    regBuf = iC_emalloc(REQUEST);
    iC_outBuf = iC_emalloc(REQUEST);
    rpyBuf = iC_emalloc(REPLY);
#endif	/* EFENCE */
    iC_initIO();			/* catch memory access signal */

    if (iC_outFP && iC_outFP != stdout) {
	fclose(iC_outFP);
#ifndef LOAD
	if (iFlag) {
	    iC_inversionCorrection();	/* only applies to compiler listing */
	    iFlag = 0;
	}
#endif	/* LOAD */
    }
    iC_outFP = stdout;			/* standard output from here */
    if (iC_errFP && iC_errFP != stderr) {
	fclose(iC_errFP);
    }
    iC_errFP = stderr;			/* standard error from here */
    if (iC_micro) iC_microReset(0);	/* start timing */

    if ((gp = iC_TX0p) != 0) {		/* are EOI or TX0 timers programmed */
	tim = gp->gt_list;		/* TX0.0 - TX0.7 */
	assert(tim);
	for (cnt = 1; cnt < 8; cnt++) {
	    if (tim[cnt] != NULL) {	/* any of the 7 timers programmed ? */
		if (cnt < TLIMIT) {
		    fprintf(iC_errFP, "\n%s: Timer TX0.%d is not supported\n", iC_iccNM, cnt);
		    iC_quit(SIGUSR1);
		}
		toCntp = &toCnt;	/* activate select timeout */
		break;			/* could optimise by varying timeout value */
	    }
	}
    }

    if ((iC_debug & 0400) == 0) {
	char *		tbp = regBuf;	/* points to next entry in regBuf */
	int		tbc = REQUEST;	/* length of filled entries */
	char *		tbt;		/* temp pointer */
	Gate **		sopp;
	char		entry[ENTRYSZ];	/* entry "SIX0000-000\0" max length 12 */
	char *		ep  = entry;	/* points to next point in entry */
	int		el  = 0;	/* entry length */
	const char *	sr[] = { "N", ",SC", ",RD", };	/* name, controller, debugger */

	/* Start TCP/IP communication before any inputs are generated => outputs */
	sockFN = iC_connect_to_server(iC_hostNM, iC_portNM);
	if (iC_debug & 04) fprintf(iC_outFP, "%s: I/O registration objects\n", iC_iccNM);
	for (i = 0; i < 3; i++) {
	    tbp += el;
	    tbc -= el;
	    el = snprintf(tbp, tbc, "%s%s", sr[i], iC_iccNM);	/* directly into regBuf */
	}
	/* use last string to initialise name "D<progname>[-<instance>]" for D_gate */
	D_gate.gt_ids = iC_emalloc(el-1);			/* including '\0' */
	strncpy(D_gate.gt_ids, tbp+2, el-1);		/* +2 leave out ",R" */
	D_gate.gt_new = -2;	/* changes - not equal 0 - 5 for debug messages */
	D_gate.gt_old = -1;	/* never changes - not equal 0 - 5 for debug messages */

	for (opp = sopp = sTable; opp < sTend; opp++) {
	    int mask;
	    gp = *opp;
	    tbp += el;
	    tbc -= el;
	    ep = entry;
	    el = 0;
	    mask = gp->gt_mark;
	    if (gp->gt_ini == -INPW && *gp->gt_ids != 'T') {	/* suppress TX0 */
		el = snprintf(ep, ENTRYSZ, ",R%s", gp->gt_ids);	/* read input at controller */
		if (gp->gt_fni != TRAB) {
		    mask = 0x100;	/*block output of used bits for analog I/O */
		}
	    } else
	    if (gp->gt_fni == OUTW) {
		el = snprintf(ep, ENTRYSZ, ",S%s", gp->gt_ids);	/* send output at controller */
		if (gp->gt_ini == -ARN) {
		    if ((tbt = strrchr(ep, '_')) != NULL &&
			*(tbt+1) == '0' &&
			*(tbt+2) == '\0') {
			*tbt = '\0';	/* strip trailing "_0" from internal output name */
			el -= 2;
			mask = 0x100;	/*block output of used bits for analog I/O */
			assert(el > 0);
		    } else {
			fprintf(iC_errFP, "\n%s: ill formed output name '%s'\n", iC_iccNM, ep + 1);
			iC_quit(SIGUSR1);
		    }
		}
	    }
	    if (el > 0) {
		char * scp = "";
		if (strlen(iC_iidNM) > 0) {
		    el += snprintf(ep + el, ENTRYSZ - el, "-%s", iC_iidNM);	/* append instance ID */
		    scp = "-";
		}
		if (iC_debug & 04) fprintf(iC_outFP,
		    "%s%s%s	%d	%d	(%d)\n", gp->gt_ids, scp, iC_iidNM, (int)gp->gt_ini, (int)gp->gt_fni, mask);
		assert(mask);
		if (mask < X_MASK) {
		    el += snprintf(ep + el, ENTRYSZ - el, "(%d)", mask);	/* mask of used bits */
		}
		if (tbc - el <= (opp+1 == sTend ? 2 : 0)) {	/* leave room for ",Z" at end */
		    /********************************************************************
		     * buffer is about to overflow
		     *******************************************************************/
		    if (iC_micro & 06) iC_microPrint("partial registration", 0);
		    regAck(sopp, opp);		/* send/receive partial registration */
		    sopp = opp;
		    tbp = regBuf;		/* start new string */
		    tbc = REQUEST;		/* length of filled entries */
		    assert(*ep == ',' && (*(ep+1) == 'S' || *(ep+1) == 'R'));
		    ep++;			/* chop initial ',' */
		    el--;
		}
		strncpy(tbp, ep, tbc);		/* place entry in regBuf */
	    }
	}
	strncpy(tbp + el, ",Z", tbc - el);	/* place termination in regBuf - 2 bytes are free */
	if (iC_micro & 06) iC_microPrint("last registration", 0);
	assert(opp == sTend);
	regAck(sopp, opp);			/* send/receive last registration */
	regOffset = snprintf(regBuf, REQUEST, "%hu:2;%s", C_channel, iC_iccNM);
	if (iC_micro & 06) iC_microPrint("Send application name", 0);
	iC_send_msg_to_server(sockFN, regBuf);			/* Application Name */
    }

/********************************************************************
 *
 *	Initialise the work lists to empty lists
 *
 *******************************************************************/

    iC_error_flag = 0;
    alist0.gt_rptr = iC_a_list = &alist1;	/* initialise alternate */
    Out_init(iC_a_list);
    alist1.gt_rptr = iC_a_list = &alist0;	/* start with alist0 */
    Out_init(iC_a_list);
    olist0.gt_rptr = iC_o_list = &olist1;	/* initialise alternate */
    Out_init(iC_o_list);
    olist1.gt_rptr = iC_o_list = &olist0;	/* start with olist0 */
    Out_init(iC_o_list);
#ifdef	LOAD
    iC_c_list = &iClock;				/* system clock list */
    Out_init(iC_c_list);
#endif	/* LOAD */
    Out_init(iC_f_list);
    Out_init(iC_s_list);
#if	YYDEBUG
    if (iC_debug & 0100) fprintf(iC_outFP, "\nINITIALISATION\n");
#endif	/* YYDEBUG */

    /* if (iC_debug & 0400) == 0 then no live bits are set in gt_live | 0x8000 */
    /* header for live data */
    /********************************************************************
     *  Initialise msgBuf for live data collection before initialisation
     *  liveOffset may also be used with messages in regBuf because they
     *  use the same channel number and all relevant messages have only
     *  one command digit.
     *  Message in msgBuf is ignored of iC_debug&0400 stops process after init
     *******************************************************************/
    msgOffset = liveOffset = snprintf(msgBuf, REQUEST, "%hu:3", C_channel);
    /********************************************************************
     *  Carry out 4 Passes to initialise all Gates
     *******************************************************************/
    for (pass = 0; pass < 4; pass++) {
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, "\n== Pass %d:", pass + 1);
#endif	/* YYDEBUG */
	for (opp = sTable; opp < sTend; opp++) {
	    gp = *opp;
	    typ = gp->gt_ini > 0 ? AND : -gp->gt_ini;
	    if (typ < MAX_OP) {
		init_fa = iC_i_lists[typ][pass];
		(*init_fa)(gp, typ);	/* initialise for this pass */
	    }
	}
    }
    iC_osc_max = iC_osc_lim;		/* during Init oscillations were not checked */

#if	YYDEBUG
    if (iC_debug & 0100) {
	fprintf(iC_outFP, "\n== Init complete =======\n");
    }
#endif	/* YYDEBUG */

    if (iC_error_flag) {
	if (iC_error_flag >= 2) {
	    fprintf(iC_outFP, "\n*** Fatal Errors ***\n");
	    iC_quit(SIGUSR1);
	}
	fprintf(iC_outFP, "\n*** Warnings ***\n");
    }
    if (iC_debug & 0400) {
	iC_quit(0);			/* terminate - no inputs */
    }
    signal(SIGINT, iC_quit);		/* catch ctrlC and Break */

#ifdef	SIGTTIN
    /********************************************************************
     *  The following behaviour was observed on Linux kernel 2.2
     *  When this process is running in the background, and a command
     *  (even a command which does not exist) is run from the bash from
     *  the same terminal on which this process was started, then this
     *  process is sent signal SIGTTIN. If the signal SIGTTIN is ignored,
     *  then stdin receives EOF (a few thousand times) every time a
     *  character is entered at the terminal.
     *
     *  To prevent this process from hanging when run in the background,
     *  SIGTTIN is ignored, and when the first EOF is received on stdin,
     *  FD_CLR(0, &infds) is executed to stop selects on stdin from then on.
     *  This means, that stdin can still be used normally for the q, t, x, d
     *  and m command when the process is run in the foreground, and behaves
     *  correctly (does not STOP mysteriously) when run in the background.
     *
     *  This means that such a process cannot be stopped with q, only with
     *  ctrl-C, when it has been brought to the foreground with fg.
     *******************************************************************/
    signal(SIGTTIN, SIG_IGN);		/* ignore tty input signal in bg */
#endif	/* SIGTTIN */

    /********************************************************************
     *  Generate a VCD file (Value Change Dump), an industry standard
     *  file format specified by IEEE-1364 (initially developed for Verilog).
     *  It is used to display storage oscilloscope type traces of bit
     *  and int signals from a running iC application with GTKWAVE.
     *  Also generate a SAV file, which has information about the traces
     *  to display initially. Inverted ALIASES are handled correctly.
     *******************************************************************/
    if (iC_vcd) {
	time_t walltime;
	char *	savCode = "";
	int	extended = 0;
	int	extFlag = 0;
	int	len;
	char *	dumpvars;
	char *	dumpPtr;
	char *	dumpEnd;
#if	INT_MAX == 32767 && defined (LONG16)
	long		value;
#else	/* INT_MAX == 32767 && defined (LONG16) */
	int		value;
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	char	modName[256];
	char	modPrev[256] = "";

	time(&walltime);
	if ((iC_vcdFP = fopen(iC_vcd, "w")) == NULL) {
	    fprintf(iC_errFP, "\n%s: cannot open vcd file '%s'\n", iC_iccNM, iC_vcd);
	    perror("fopen");
	    iC_quit(SIGUSR1);
	}
	/********************************************************************
	 *  Generate a SAV file if the VCD file has the extension '.vcd'.
	 *  Output standard headers for both the SAV and the VCD file.
	 *******************************************************************/
	if ((cp = strrchr(iC_vcd, '.')) != NULL && strcmp(++cp, "vcd") == 0) {
	    iC_sav = iC_emalloc(strlen(iC_vcd)+1);	/* +1 for '\0' */
	    strcpy(iC_sav, iC_vcd);
	    cp = strrchr(iC_sav, '.') + 1;
	    strcpy(cp, "sav");		/* genrate xxx.sav only if xxx.vcd */
	    if ((iC_savFP = fopen(iC_sav, "w")) == NULL) {
		fprintf(iC_errFP, "\n%s: cannot open sav file '%s'\n", iC_iccNM, iC_sav);
		perror("fopen");
		iC_quit(SIGUSR1);
	    }
	    /********************************************************************
	     *  initial SAV header
	     *******************************************************************/
	    fprintf(iC_savFP,
		"[*]\n"
		"[*] immediate C %s\n"
		"[*] %s"
		"[*]\n"
		"[dumpfile] \"%s\"\n"
		"[size] 1024 700\n"
		"[pos] -1 -1\n"
		"*-4.343955 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1\n"
	    , iC_ID, asctime(gmtime(&walltime)), iC_iccNM);	/* zoom, tims.marker, 26 named_markers */
	    savCode = sav_ftype[GATE];	/* "wire" instead of "event" for iClock */	
	    fprintf(iC_savFP,
		"@%s\n"
		"%s.iClock\n"		/* display iClock as the first trace */
	    , savCode, iC_iccNM);
	}
	/********************************************************************
	 *  initial VCD header
	 *******************************************************************/
	fprintf(iC_vcdFP,
	    "$date\n"
	    "    %s"
	    "$end\n"
	    "$version\n"
	    "    immediate C %s\n"
	    "$end\n"
	    "$timescale\n"
	    "     1 us\n"
	    "$end\n"
	    "\n"
	    "$scope module %s $end\n"
	, asctime(gmtime(&walltime)), iC_ID, iC_iccNM);
	/********************************************************************
	 *  prepare index entries first to allow ALIAS back-references
	 *******************************************************************/
	index = 0;
	for (opp = sTable; opp < sTend; opp++) {
	    if (index >= 10000) {
		fprintf(iC_errFP, "\n%s: cannot handle more than 10,000 variables in file '%s'\n", iC_iccNM, iC_vcd);
		iC_quit(SIGUSR1);
	    }
	    gp = *opp;
	    if (
#ifdef	LOAD
		gp == &iClock
#else	/* LOAD */
		strcmp(gp->gt_ids, "iClock") == 0
#endif	/* LOAD */
	    ) {
		iClock_index = index;	/* must be set before next loop in case first symbol comes before iClock */
	    }
	    gp->gt_live = index++;	/* index | live inhibit 0x8000 set to 0x0000 */
	}
	liveIndex = 1;
	dumpPtr = dumpvars = iC_emalloc(index * 8);	/* enough for all integer variables with 4 byte codes */
	dumpEnd = dumpvars + (index * 7);		/* will be free'd in this routine */
	/********************************************************************
	 *  to maintain index correlation handle all symbols
	 *******************************************************************/
	for (opp = sTable; opp < sTend; opp++) {
	    int		fni;
	    Gate *	gm;
	    char *	ids;
	    char *	idsa;
	    char *	code;
	    int		size;
	    char *	prefix;
	    char	iqt[2];		/* single char buffer - space for 0 terminator */
	    char	xbwl[2];	/* single char buffer - space for 0 terminator */
	    int		byte;
	    int		bit;
	    char	tail[15];	/* compiler generated suffix .123456_123456 max */
	    int		inverse = 0;

	    gp = *opp;
	    if (gp->gt_ini != -NCONST) {		/* do not trace numbers */
		idsa = ids = gp->gt_ids;
		fni = gp->gt_fni;
		if (fni == UDFA && (gp->gt_ini == -ARNC || gp->gt_ini == -LOGC)) {
		    /********************************************************************
		     *  Resolve immC array - the array references are like ALIAS'es.
		     *  Treat the array like a module, whose members are all the array
		     *  reference names recorded in the VCD file. They are not mentioned
		     *  in the SAV file. But they can be selected for display later.
		     *******************************************************************/
		    if (extFlag) {
			fprintf(iC_vcdFP, "$upscope $end\n");
			extFlag = 0;
		    }
		    fprintf(iC_vcdFP, "$scope module %s $end\n", ids);	/* use array identifier as module name */
		    for (i = 0; i < gp->gt_old; i++) {
			gm = gp->gt_rlist[i];		/* immC member - cannot be an ALIAS itself */
			fni = MAX_FTY + gm->gt_fni;	/* match iClive */
			if ((code = vcd_ftype[fni]) != NULL) {
			    size = *code == 'i' ? 32 : 1;
			    fprintf(iC_vcdFP, "$var %s %d %d %s[%d] $end\n", code, size, gm->gt_live, ids, i);
			}
		    }
		    fprintf(iC_vcdFP, "$upscope $end\n");
		} else {
		    /********************************************************************
		     *  Resolve ALIAS'es
		     *******************************************************************/
		    while (gp->gt_ini == -ALIAS) {
			if (gp->gt_fni == GATE || gp->gt_fni == GATEX) {
			    inverse ^= gp->gt_mark;	/* holds ALIAS inversion flag */
			}
			gp = gp->gt_rptr;		/* resolve ALIAS */
			idsa = gp->gt_ids;		/* alias name */
			fni = MAX_FTY + gp->gt_fni + (inverse << 1);	/* match iClive */
		    }
		    /********************************************************************
		     *  Ignore variables whose name finishes with _0. They are compiler
		     *  generated output variables, which always follow their base variable.
		     *  Determine extended variables, whose name finish with _1 _2 etc.
		     *  These are generated auxiliary variables, which are not displayed
		     *  initially. They are grouped in modules, which have the base name
		     *  of these variables and which may be selected for display later.
		     *******************************************************************/
		    extended = 0;
		    if ((cp = strrchr(ids, '_')) != NULL) {
			len = cp - ids;		/* length of base for generating module */
			if (len > 255) len = 255;
			if (strcmp(++cp, "0") == 0) continue;	/* ignore Qnn_0 */
			extended = 1;		/* possibly an extended iC variable xxx_1 */
			while (*cp) {
			    if (! isdigit(*cp++)) {
				extended = 0;	/* not an extended iC variable xxx_a */
				break;
			    }
			}
		    }
		    /********************************************************************
		     *  Generate a VCD $var line for each displayable variable, which
		     *  associates the name of the variable with its display type, size
		     *  and ID code. Although the ID code can be 1 to 4 bytes of any
		     *  printable ASCII character, in this immdiate C implementation
		     *  the ID codes chosen  are made up of 1 to 4 decimal digits only
		     *  and are in fact the index offset of the internal symbol table
		     *  stored in gt_live of each gate node for the live display in iClive.
		     *  Extended variables are bracketed by a $sope module and a $upscope
		     *  line.
		     *******************************************************************/
		    if ((code = vcd_ftype[fni]) != NULL) {
			if (gp->gt_live == iClock_index) {
			    code = vcd_ftype[GATE];	/* "wire" instead of "event" for iClock */	
			}
			if (extended) {
			    strncpy(modName, ids, len);
			    *(modName + len) = '\0';	/* generate substring without extension */
			    if ((cp = strrchr(modName, '.')) != 0) {
				*cp = '_';		/* modify QX0.0 to QX0_0 for module name */
			    }
			    if (strcmp(modPrev, modName) != 0) {
				if (extFlag) {
				    fprintf(iC_vcdFP, "$upscope $end\n");	/* change in module name */
				}
				fprintf(iC_vcdFP, "$scope module %s $end\n", modName);
				extFlag = 1;
				strncpy(modPrev, modName, 256);
			    }
			} else if (extFlag) {
			    fprintf(iC_vcdFP, "$upscope $end\n");
			    extFlag = 0;
			}
			prefix = "";
			if (sscanf(ids, "%1[IQT]%1[XBWL]%5d.%5d%7s", iqt, xbwl, &byte, &bit, tail) == 4) {
			    size = 1;
			    prefix = "\\";		/* variables containing '.' must be prefixed with '\' in .sav file */
			} else
			if (sscanf(idsa, "%1[IQT]%1[BWL]%5d%7s", iqt, xbwl, &byte, tail) >= 3 &&
			    (! extended || strcmp(tail, "_0") == 0)) {
			    switch (xbwl[0]) {
			    case 'B':
				size = 8;		/* reduce size of QBn although only QBn_0 output is reduced for output */
				break;		/* NOTE: ALIAS QBn and QWN has size of aliased variable - usually 32 */
			    case 'W':
				size = 16;		/* reduce size of QWn although only QWn_0 output is reduced for output */
				break;
#if INT_MAX != 32767 || defined (LONG16)
			    case 'L':
				size = 32;
				break;
#endif	/* INT_MAX != 32767 || defined (LONG16) */
			    default:
				assert(0);		/* no valid output word definition */
			    }
			} else {
			    size = *code == 'i' ? 32 : 1;
			}
			fprintf(iC_vcdFP, "$var %s %d %d %s $end\n", code, size, gp->gt_live, ids);
			/********************************************************************
			 *  Generate $dumpvar initialisation string from values produced in
			 *  Pass 1 - Pass 4 above. This starts traces of inverted logic and
			 *  initial arithmetic values correctly.
			 *******************************************************************/
			if (fni < MAX_FTY) {			/* no initial $dumpvar for ALIAS */
			    switch (*code) {
			    case 'e':				/* event - this still shows as a '1' tick despite value '0' */
			    case 'w':				/* wire */
				assert(dumpPtr < dumpEnd - 5);	/* enough for one more entry */
				dumpPtr += sprintf(dumpPtr, " %d%hu", gp->gt_val < 0 ? 1 : 0, gp->gt_live);
				break;
			    case 'i':				/* integer */
				assert(dumpPtr < dumpEnd - 38);	/* enough for one more entry */
				convert2binary(binBuf, gp->gt_new, 0);	/* convert to 32 bit binary string */
				dumpPtr += sprintf(dumpPtr, " b%s %hu", binBuf, gp->gt_live);
				break;
			    default:				/* hard error */
				assert(0);
				break;
			    }
			}
			/********************************************************************
			 *  For the SAV file generate a line for each variable to be displayed
			 *  initially. Precede each group of variables of the same type by the
			 *  SAV types @24 for a decimal integer, @ 28 and @ 68 for normal and
			 *  inverted bit types as well as @28 for clock and timer events.
			 *******************************************************************/
			if (iC_savFP && ! extended && (code = sav_ftype[fni]) != NULL && gp->gt_live != iClock_index) {
			    if (strcmp(savCode, code) != 0) {
				fprintf(iC_savFP, "@%s\n", code);
				savCode = code;
			    }
			    if (size == 1) {
				fprintf(iC_savFP, "%s.%s%s\n", iC_iccNM, prefix, ids);
			    } else {
				fprintf(iC_savFP, "%s.%s[%d:0]\n", iC_iccNM, ids, size - 1);
			    }
			}
		    }
		}
	    }
	}
	if (extFlag) {
	    fprintf(iC_vcdFP, "$upscope $end\n");
	}
	fprintf(iC_vcdFP,
	    "$upscope $end\n"					/* finalise initial $scope */
	    "$enddefinitions $end\n"
	    "\n"
	);
	fprintf(iC_vcdFP, "$dumpvars%s $end\n", dumpvars);	/* finally print the $dumpvars string */
	free(dumpvars);
    } /* end of VCD SAV initialisation */

    if ((gp = tim[0]) != NULL) {
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, "EOI:\t%s  1 ==>", gp->gt_ids);
#endif	/* YYDEBUG */
	gp->gt_val = -1;		/* set EOI once as first action */
	iC_liveData(gp, 1);		/* VCD and/or iClive */
	iC_link_ol(gp, iC_o_list);	/* fire EOI Input Gate */
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, " -1\n");
#endif	/* YYDEBUG */
    }

    /********************************************************************
     *  The following initialisation function is an empty function
     *  in the libict.a support library.
     *		int iCbegin(void) { return 0; }
     *  It may be implemented in a literal block of an iC source, in
     *  which case that function will be linked in preference.
     *  User implementations of iCbegin() should return 1, to activate
     *  the debug message "iCbegin complete ====".
     *
     *  It can be used to initialise immC variables etc. For this reason
     *  it should be executed once after EOI, but before normal processing.
     *  It can use the initial values of all immediate variables.
     *
     *  If the iCbegin() function contains a fork() call, a child process is
     *  spawned, which will run in parallel with immediate processing.
     *******************************************************************/
    if (iCbegin()) {			/* initialisation function */
#if	YYDEBUG
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\niCbegin complete ====\n");
	}
#endif	/* YYDEBUG */
    }
    /********************************************************************
     *  Operational loop
     *******************************************************************/
    for (;;) {
	/********************************************************************
	 *  Sequencing of different action lists and New I/O handling
	 *
	 *  1   initialisation - put EOI on o_list
	 *      # actions after an idle period:
	 *  2   Loop:  ++mark_stamp to control oscillations
	 ****** COMBINATORIAL PHASE *****
	 *             scan a_list unless a_list empty
	 *                 INPW ARITH expr results to a_list
	 *                 comparisons, &&, || to o_list
	 *                 clocked actions to c_list via own clock list
	 *  3        { scan o_list; goto Loop } unless o_list empty
	 *                 bit actions to o_list
	 *                 bits used in arithmetic to a_list (less common)
	 *                 clocked actions to c_list via own clock list
	 ****** CLOCK PHASE *******
	 *  4        { ++mark_stamp to control oscillations
	 *             scan c_list; DO 5; goto Loop } unless c_list empty
	 *                 transfer ARITH master values as slave values to a_list
	 *                 transfer GATE master values as slave values to o_list
	 *                 (does not use any combinatorial ARITH or GATE values)
	 *                 transfer master entries on slave clock lists to c_list
	 *                 (continue scanning c_list until all these have been handled)
	 *                 defer 'if else switch' slave C actions to f_list
	 ****** COMBINATORIAL PHASE *****
	 *  5        { scan f_list; } unless f_list empty
	 *                 C actions can use and generate combinatotrial ARITH and
	 *                 GATE values, which is start of a new combinatorial scan
	 *  6   scan s_list			# only one scan is required
	 *          do OUTW and OUTX Gates building send string from channels
	 *  7   send output string with final outputs only
	 *  8   switch to alternate a_list and o_list
	 *  9   IDLE - wait for next input
	 * 10   read new input and link INPW Gates directly to a_list
	 *      or via traMb to o_list; goto Loop
	 *******************************************************************/
	if (++iC_mark_stamp == 0) {	/* next generation for oscillator check */
	    iC_mark_stamp++;		/* leave out zero */
	}
	for (vcdFlag = iC_vcdFP;;) {
	    if (iC_a_list != iC_a_list->gt_next) { iC_scan_ar (iC_a_list);           }
	    if (iC_o_list != iC_o_list->gt_next) { iC_scan    (iC_o_list); continue; }
	    if (iC_c_list != iC_c_list->gt_next) {
		if (++iC_mark_stamp == 0) {	/* next generation for oscillator check */
		    iC_mark_stamp++;		/* leave out zero */
		}
		if (iC_vcdFP) {
		    fprintf(iC_vcdFP, "#%ld\n1%hu\n", ++virtualTime, iClock_index);	/* mark active iClock in VCD output */
		    vcdFlag = NULL;		/* no 2nd empty clock scan vcd output */
		}
		iC_scan_clk(iC_c_list);		/* new flist entries can only occurr here */
		if (iC_vcdFP) {
		    fprintf(iC_vcdFP, "#%ld\n0%hu\n", ++virtualTime, iClock_index);	/* end active iClock in VCD output */
		}
		if (iC_f_list != iC_f_list->gt_next) { iC_scan_clk(iC_f_list); }
		continue;
	    } else if (vcdFlag) {
		fprintf(iC_vcdFP, "#%ld\n1%hu\n", ++virtualTime, iClock_index);	/* mark non active iClock in VCD output */
		fprintf(iC_vcdFP, "#%ld\n0%hu\n", ++virtualTime, iClock_index);	/* end non active iClock in VCD output */
	    }
	    if (iC_s_list != iC_s_list->gt_next) { iC_scan_snd(iC_s_list);           }
	    break;
	}

	/********************************************************************
	 *  Send live data collected in msgBuf during initialisation
	 *  and previous loop to iCserver
	 *******************************************************************/
	if (msgOffset > liveOffset) {
	    iC_send_msg_to_server(sockFN, msgBuf);
	}
	/********************************************************************
	 *  Initialise msgBuf for live data collection during next loop
	 *******************************************************************/
	msgOffset = liveOffset;				/* msg = "C_channel:3" */

#if	YYDEBUG
	if (iC_scan_cnt || iC_link_cnt) {
	    if (iC_debug & 0100) fprintf(iC_outFP, "\n");
	    if (iC_debug & 02000) {
		fprintf(iC_outFP, "scan = %5u  link = %5u",
		    iC_scan_cnt, iC_link_cnt);
		if (iC_glit_cnt) {
		    fprintf(iC_outFP, "  glitch = %5u, %lu", iC_glit_cnt, glit_nxt);
		}
		fprintf(iC_outFP, "\n");
	    }
	    iC_glit_cnt = glit_nxt =
	    iC_scan_cnt = iC_link_cnt = 0;
	}
#endif	/* YYDEBUG */
	/********************************************************************
	 *  Switch to alternate lists
	 *
	 *  alternate list contains all those gates which were marked in
	 *  the previous scan and which were active more than
	 *  MARKMAX times. These are oscillators which wil be
	 *  scanned again in the next cycle.
	 *******************************************************************/
	iC_a_list = iC_a_list->gt_rptr;	/* alternate arithmetic list */
	iC_o_list = iC_o_list->gt_rptr;	/* alternate logic list */

	/********************************************************************
	 *  Send output data collected in outBuf to iCserver
	 *******************************************************************/
	if (iC_outOffset > 1) {		/* if any item it is at least 2 long 'x,' */
	    iC_outBuf[iC_outOffset - 1] = '\0';	/* clear last ',' */
	    iC_send_msg_to_server(sockFN, iC_outBuf);
	    memset(iC_outBuf, 0, REQUEST);	/* clear for next data from iC_outMw() */
	    iC_outOffset = 0;
	}
	if (iC_micro) iC_microPrint("Scan complete", 0);

	if (iC_osc_gp) {
	    fprintf(iC_outFP,
		"*** Warning: %s has oscillated %d times - check iC program!!!\n",
		iC_osc_gp->gt_ids, iC_osc_gp->gt_mcnt);
	    iC_osc_gp = NULL;
	}

#if	YYDEBUG
	if (iC_debug & 0300) {		/* osc or detailed info */
	    if ((iC_debug & 0200) &&
		(iC_a_list->gt_next != iC_a_list || iC_o_list->gt_next != iC_o_list)) {
		fprintf(iC_outFP, "OSC =");
		for (gp = iC_a_list->gt_next; gp != iC_a_list; gp = gp->gt_next) {
		    fprintf(iC_outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
		}
		for (gp = iC_o_list->gt_next; gp != iC_o_list; gp = gp->gt_next) {
		    fprintf(iC_outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
		}
		fprintf(iC_outFP, "\n");
	    }
	    fprintf(iC_outFP, "======== WAIT ==========\n");
	    fflush(iC_outFP);
	}
#endif	/* YYDEBUG */

	/********************************************************************
	 *  Input from external input modules and time input (if used)
	 *  Wait for input in a select statement most of the time
	 *******************************************************************/
	if (iC_vcdFP) {
	    virtualTime += 11;
	    virtualTime -= (virtualTime%10 + 1);	/* next input is on 10's boundary */
	}
	for (cnt = 0; cnt == 0; ) {		/* stay in input loop if nothing linked */
	    retval = iC_wait_for_next_event(sockFN, iC_osc_flag ? &toCnt : toCntp);	/* wait for inputs or timer */
	    if (iC_osc_flag) {
		cnt++;				/* gates have been linked to alternate list */
		iC_osc_flag = 0;		/* normal timer operation again */
	    }

	    if (retval == 0) {
		if (toCnt.tv_sec == 0 && toCnt.tv_usec == 0) {
		    toCnt = iC_timeOut;		/* transfer timeout value */
		}
		/********************************************************************
		 *  TIMERS here every 50 milliseconds - ~54 ms for MSDOS
		 *
		 *  The iC_debug facility -d1000 stops linking the 100 ms to 60 second
		 *  TIMERS tim[4] to tim[7] when they are connected directly to
		 *  the slave input of a clock or timer Gate and that clock or timer
		 *  has no Gates on the clock list it controls - ie nothing will happen.
		 *  This stops continuous output when tracing logic (-d100 or -t) when
		 *  these internal TIMERS produce no change.
		 *  It also reduces the data traffic to iClive and reduces flashing of
		 *  clocks and timers controlled by internal TIMERS.
		 *******************************************************************/
		if ((gp = tim[4]) != 0) {		/* 100 millisecond timer */
#if	YYDEBUG
		    if (iC_debug & 01000) {
			Gate **	lp;
			Gate *	tp;
			int	cn1 = 2;
			lp = gp->gt_list;		/* test if any clock or timer is active */
			do {				/* for normal and inverted */
			    while ((tp = *lp++) != 0) {
				if (tp->gt_fni == CLCK || tp->gt_fni == TIMR) {
				    tp = tp->gt_funct;
				    if (tp->gt_next != tp) {
					goto linkT4;	/* found an active clock or timer */
				    }
				} else {
				    goto linkT4;	/* found a link to non clock or timer */
				}
			    }
			} while (--cn1);
			goto skipT4;			/* excuse spaghetti - faster without flag */
		    }
		linkT4:
		    if (iC_micro && !cnt) iC_microPrint("Timer received", 0);
		    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
		    gp->gt_val = - gp->gt_val;		/* complement input */
		    iC_liveData(gp, gp->gt_val < 0 ? 1 : 0);	/* VCD and/or iClive */
		    iC_link_ol(gp, iC_o_list);
		    cnt++;
#if	YYDEBUG
		    if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
		skipT4: ;
#endif	/* YYDEBUG */
		}
		if (--tcnt1 <= 0) {
		    tcnt1 = D10;			/* 10 under Linux, 9 under MSDOS */
		    if ((gp = tim[5]) != 0) {		/* 1 second timer */
#if	YYDEBUG
			if (iC_debug & 01000) {
			    Gate **	lp;
			    Gate *	tp;
			    int	cn1 = 2;
			    lp = gp->gt_list;		/* test if any clock or timer is active */
			    do {			/* for normal and inverted */
				while ((tp = *lp++) != 0) {
				    if (tp->gt_fni == CLCK || tp->gt_fni == TIMR) {
					tp = tp->gt_funct;
					if (tp->gt_next != tp) {
					    goto linkT5;	/* found an active clock or timer */
					}
				    } else {
					goto linkT5;	/* found a link to non clock or timer */
				    }
				}
			    } while (--cn1);
			    goto skipT5;		/* excuse spaghetti - faster without flag */
			}
		    linkT5:
			if (iC_micro && !cnt) iC_microPrint("Timer received", 0);
			if (iC_debug & 0100) fprintf(iC_outFP, "\n%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
			gp->gt_val = - gp->gt_val;	/* complement input */
			iC_liveData(gp, gp->gt_val < 0 ? 1 : 0);	/* VCD and/or iClive */
			iC_link_ol(gp, iC_o_list);
			cnt++;
#if	YYDEBUG
			if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
		    skipT5: ;
#endif	/* YYDEBUG */
		    }
		    if (--tcnt10 <= 0) {
			tcnt10 = 10;
			if ((gp = tim[6]) != 0) {	/* 10 second timer */
#if	YYDEBUG
			    if (iC_debug & 01000) {
				Gate **	lp;
				Gate *	tp;
				int	cn1 = 2;
				lp = gp->gt_list;	/* test if any clock or timer is active */
				do {			/* for normal and inverted */
				    while ((tp = *lp++) != 0) {
					if (tp->gt_fni == CLCK || tp->gt_fni == TIMR) {
					    tp = tp->gt_funct;
					    if (tp->gt_next != tp) {
						goto linkT6;/* found an active clock or timer */
					    }
					} else {
					    goto linkT6;/* found a link to non clock or timer */
					}
				    }
				} while (--cn1);
				goto skipT6;		/* excuse spaghetti - faster without flag */
			    }
			linkT6:
			    if (iC_micro && !cnt) iC_microPrint("Timer received", 0);
			    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
			    gp->gt_val = - gp->gt_val;	/* complement input */
			    iC_liveData(gp, gp->gt_val < 0 ? 1 : 0);	/* VCD and/or iClive */
			    iC_link_ol(gp, iC_o_list);
			    cnt++;
#if	YYDEBUG
			    if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
			skipT6: ;
#endif	/* YYDEBUG */
			}
			if (--tcnt60 <= 0) {
			    tcnt60 = 6;
			    if ((gp = tim[7]) != 0) {	/* 60 second timer */
#if	YYDEBUG
				if (iC_debug & 01000) {
				    Gate **	lp;
				    Gate *	tp;
				    int	cn1 = 2;
				    lp = gp->gt_list;	/* test if any clock or timer is active */
				    do {		/* for normal and inverted */
					while ((tp = *lp++) != 0) {
					    if (tp->gt_fni == CLCK || tp->gt_fni == TIMR) {
						tp = tp->gt_funct;
						if (tp->gt_next != tp) {
						    goto linkT7;/* found an active clock or timer */
						}
					    } else {
						goto linkT7;/* found a link to non clock or timer */
					    }
					}
				    } while (--cn1);
				    goto skipT7;	/* excuse spaghetti - faster without flag */
				}
			    linkT7:
				if (iC_micro && !cnt) iC_microPrint("Timer received", 0);
				if (iC_debug & 0100) fprintf(iC_outFP, "\n%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
				gp->gt_val = - gp->gt_val;	/* complement input */
				iC_liveData(gp, gp->gt_val < 0 ? 1 : 0);	/* VCD and/or iClive */
				iC_link_ol(gp, iC_o_list);
				cnt++;
#if	YYDEBUG
				if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
			    skipT7: ;
#endif	/* YYDEBUG */
			    }
			}
		    }
		}
	    } else if (retval > 0) {
		if (FD_ISSET(sockFN, &iC_rdfds)) {
		    /********************************************************************
		     *  TCP/IP input
		     *******************************************************************/
		    if (iC_rcvd_msg_from_server(sockFN, rpyBuf, REPLY)) {
#if	INT_MAX == 32767 && defined (LONG16)
			long		val;
#else	/* INT_MAX == 32767 && defined (LONG16) */
			int		val;
#endif	/* INT_MAX == 32767 && defined (LONG16) */
			unsigned short	channel;
			static int	liveFlag;

			if (iC_micro && !cnt) iC_microPrint("Input received", 0);
			if (isdigit(rpyBuf[0])) {
			    assert(Channels);
			    cp = rpyBuf; cp--;
			    do {
				int n;
				if (
#if	INT_MAX == 32767 && defined (LONG16)
				    (n = sscanf(++cp, "%hu:%ld", &channel, &val))
#else	/* INT_MAX == 32767 && defined (LONG16) */
				    (n = sscanf(++cp, "%hu:%d", &channel, &val))
#endif	/* INT_MAX == 32767 && defined (LONG16) */
				    == 2 &&
				    channel > 0	&&
				    channel <= topChannel &&
				    (gp = Channels[channel]) != NULL
				) {
				    if (val != gp->gt_new &&				/* first change or glitch */
				    ((gp->gt_new = val) != gp->gt_old) ^ (gp->gt_next != 0)) {
					/* arithmetic master action */
					if (gp->gt_fni == TRAB) {
#if	YYDEBUG
#if	INT_MAX == 32767 && defined (LONG16)
					    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s<\t%hu:%ld\t0x%02lx ==>> 0x%02lx",
						gp->gt_ids, channel, gp->gt_new, gp->gt_old, gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
					    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s<\t%hu:%d\t0x%02x ==>> 0x%02x",
						gp->gt_ids, channel, gp->gt_new, gp->gt_old, gp->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
					    else if (iC_debug & 020) fprintf(iC_outFP, "%s\t%2hu:%08s\n",
						gp->gt_ids, channel, convert2binary(binBuf, gp->gt_new, 1));
#endif	/* YYDEBUG */
					    cnt += iC_traMb(gp, 0);			/* distribute bits directly */
					} else
					if (gp->gt_ini == -INPW) {
#if	YYDEBUG
					    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s[\t", gp->gt_ids);
#if	INT_MAX == 32767 && defined (LONG16)
					    if (iC_debug & 0100) fprintf(iC_outFP, "%hu:%ld\t%ld ==>", channel, gp->gt_new, gp->gt_old);
#else	/* INT_MAX == 32767 && defined (LONG16) */
					    if (iC_debug & 0100) fprintf(iC_outFP, "%hu:%d\t%d ==>", channel, gp->gt_new, gp->gt_old);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#endif	/* YYDEBUG */
					    iC_liveData(gp, gp->gt_new);		/* VCD and/or iClive */
					    iC_link_ol(gp, iC_a_list);	/* no actions */
#if	YYDEBUG
#if	INT_MAX == 32767 && defined (LONG16)
					    if (iC_debug & 0100) fprintf(iC_outFP, " %ld", gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
					    if (iC_debug & 0100) fprintf(iC_outFP, " %d", gp->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#endif	/* YYDEBUG */
					    cnt++;
					} else
					if (gp->gt_fni == UDFA) {			/* D_gate initialised type */
					    char * cp1;
					    gp->gt_new = 0;				/* allow repeated 1-6 commands */
					    switch (val) {
					    case 0:					/* IGNORE */
						break;

					    case 1:					/* GET_SYMBOL_TABLE */
#if	YYDEBUG
						if (iC_debug & 0100) fprintf(iC_outFP, "Symbol Table requested by '%s'\n", iC_iccNM);
#endif	/* YYDEBUG */
						if (! liveIndex) {
						    /* prepare index entries first to allow ALIAS back-references */
						    index = 0;
						    for (opp = sTable; opp < sTend; opp++) {
							(*opp)->gt_live = index++;	/* index and live inhibit */
							index &= 0x7fff;		/* rolls over if > 32768 Symbols */
						    }
						}
						regOffset = snprintf(regBuf, REQUEST, "%hu:1", C_channel);
						/* to maintain index correlation send all symbols */
						for (opp = sTable; opp < sTend; opp++) {
						    int		len;
						    int		rest;
						    int		fni;
						    int		i;
						    int		inverse = 0;
						    char *	ids;
						    Gate *	gm;

						    gp = *opp;
						    ids = gp->gt_ids;
						    fni = gp->gt_fni;
						    if (fni == UDFA && (gp->gt_ini == -ARNC || gp->gt_ini == -LOGC)) {
							for (i = 0; i < gp->gt_old; i++) {	/* immC array */
							    gm = gp->gt_rlist[i];	/* immC member - cannot be an ALIAS itself */
							    fni = MAX_FTY + gm->gt_fni;	/* match iClive */
							    while (rest = REQUEST - regOffset, (len =
								snprintf(&regBuf[regOffset], rest, ";%s[%d] %d %d",
								ids, i, fni, gm->gt_live)) < 0 || len >= rest) {
								regBuf[regOffset] = '\0';	/* terminate */
								if (iC_micro & 06) iC_microPrint("Send Symbols intermediate", 0);
								iC_send_msg_to_server(sockFN, regBuf);
								regOffset = liveOffset;
							    }
							    regOffset += len;
							}
						    }
						    fni = gp->gt_fni;
						    while (gp->gt_ini == -ALIAS) {
							if (gp->gt_fni == GATE || gp->gt_fni == GATEX) {
							    inverse ^= gp->gt_mark;	/* holds ALIAS inversion flag */
							}
							gp = gp->gt_rptr;		/* resolve ALIAS */
							fni = MAX_FTY + gp->gt_fni + (inverse << 1);	/* match iClive */
						    }
						    while (rest = REQUEST - regOffset, (len = fni >= MAX_FTY ?
							snprintf(&regBuf[regOffset], rest, ";%s %d %d", ids, fni, gp->gt_live) :
							snprintf(&regBuf[regOffset], rest, ";%s %d", ids, fni)
							) < 0 || len >= rest) {
							regBuf[regOffset] = '\0';	/* terminate */
							if (iC_micro & 06) iC_microPrint("Send Symbols intermediate", 0);
							iC_send_msg_to_server(sockFN, regBuf);
							regOffset = liveOffset;
						    }
						    regOffset += len;
						}
						if (regOffset > liveOffset) {
						    if (iC_micro & 06) iC_microPrint("Send Symbols", 0);
						    iC_send_msg_to_server(sockFN, regBuf);
						}
						/* end of symbol table - execute scan - follow with '0' to leave in iCserver */
						regOffset = snprintf(regBuf, REQUEST, "%hu:4,%hu:0", C_channel, C_channel);
						if (iC_micro & 06) iC_microPrint("Send Scan Command", 0);
						iC_send_msg_to_server(sockFN, regBuf);
						liveFlag = 1;				/* live inhibit bits are set */
						break;

					    case 3:					/* RECEIVE_ACTIVE_SYMBOLS */
					    case 4:					/* LAST_ACTIVE_SYMBOLS */
						if (liveFlag) {
						    for (opp = sTable; opp < sTend; opp++) {
							(*opp)->gt_live &= 0x7fff;	/* clear live active */
						    }
						    liveFlag = 0;			/* do not set again until case 4 received */
						}
						/********************************************************************
						 *  Send live data just in case there was also an input in this loop
						 *  (should go straight to scan())
						 *******************************************************************/
						if (msgOffset > liveOffset) {
						    iC_send_msg_to_server(sockFN, msgBuf);
						}
						msgOffset = liveOffset;			/* msg = "C_channel:3" */
						cp1 = rpyBuf;
						while ((cp1 = strchr(cp1, ';')) != NULL) {
						    long	value;
						    int		fni;
						    index = atoi(++cp1);
						    assert(index < sTend - sTable);	/* check index is in range */
						    gp = sTable[index];
						    gp->gt_live |= 0x8000;		/* set live active */
						    value = (
							(fni = gp->gt_fni) == ARITH ||
							fni == D_SH ||
							fni == F_SW ||
							fni == TRAB ||
							fni == CH_AR)	? gp->gt_new
									: fni == OUTW	? gp->gt_out
											: gp->gt_val < 0  ? 1
													  : 0;
						    if (iC_debug & 04) fprintf(iC_outFP, "%4hu %-15s %ld\n",
							index, gp->gt_ids, value);	/* only INT_MAX != 32767 */
						    if (value) {
							iC_liveData(gp, value);		/* initial active live values */
						    }
						}
						/********************************************************************
						 *  Send live data collected in msgBuf during RECEIVE_ACTIVE_TELEGRAMS
						 *  because scan() is only executed when another input occurs.
						 *  a scan may occurr between received blocks which
						 *  also sends a live C_channel:3 block
						 *******************************************************************/
						if (msgOffset > liveOffset) {
						    iC_send_msg_to_server(sockFN, msgBuf);
						}
						msgOffset = liveOffset;			/* msg = "C_channel:3" */
						if (val == 4) {
						    liveFlag = 1;			/* live inhibit bits are correct */
						}
						break;

					    case 2:		/* POLL */
						/********************************************************************
						 *  Receive when re-registering - makes sure, that application is seen
						 *******************************************************************/
						if (liveFlag) {
						    for (opp = sTable; opp < sTend; opp++) {
							(*opp)->gt_live &= 0x7fff;	/* clear live active */
						    }
						    liveFlag = 0;
						}
						/* poll iClive with 'ch:2;<name>' */
						regOffset = snprintf(regBuf, REQUEST, "%hu:2;%s", C_channel, iC_iccNM);
						if (iC_micro & 06) iC_microPrint("Send application name", 0);
						iC_send_msg_to_server(sockFN, regBuf);
						break;

					    case 5:					/* GET_END */
						/********************************************************************
						 *  Receive from iClive when Symbol Table is no longer required
						 *******************************************************************/
						if (liveFlag) {
						    for (opp = sTable; opp < sTend; opp++) {
							(*opp)->gt_live &= 0x7fff;	/* clear live active */
						    }
						    liveFlag = 0;
						}
						regOffset = snprintf(regBuf, REQUEST, "%hu:0", C_channel);
						iC_send_msg_to_server(sockFN, regBuf);
						break;

					    case 6:					/* STOP PROGRAM */
						iC_quit(QUIT_DEBUGGER);			/* quit normally */
						break;

					    case 7:					/* DEBUGGER STOPPED */
#if	YYDEBUG
						if (iC_debug & 0100) fprintf(iC_outFP, "Debugger has stopped for '%s'\n", iC_iccNM);
#endif	/* YYDEBUG */
						regOffset = snprintf(regBuf, REQUEST, "%hu:0", C_channel);
						iC_send_msg_to_server(sockFN, regBuf);
						break;

					    default:
						goto RcvError;				/* unknown C_channel:? case */
					    }
					} else goto RcvError;
				    }
				} else {
				    fprintf(iC_errFP, "n = %d, channel = %hu, topChannel = %hu, gp = %s\n", n, channel, topChannel, gp ? gp->gt_ids : "null");
				    goto RcvError;
				}
			    } while ((cp = strchr(cp, ',')) != NULL);
			}
			else {
			  RcvError:
			    fprintf(iC_errFP, "ERROR: '%s' rcvd at '%s' ???\n", rpyBuf, iC_iccNM);
			}
		    } else {
			iC_quit(QUIT_SERVER);		/* quit normally */
		    }
		} else if (FD_ISSET(0, &iC_rdfds)) {	/* unlikely to have both */
		    /********************************************************************
		     *  STDIN
		     *******************************************************************/
		    if ((c = getchar()) == EOF) {
			FD_CLR(0, &iC_infds);		/* ignore EOF - happens in bg */
		    } else if (c == 'q') {
			iC_quit(QUIT_TERMINAL);		/* quit normally */
#if	YYDEBUG
		    } else if (c == 't') {
			iC_debug ^= 0100;		/* toggle -t flag */
		    } else if (c == 'm') {
			iC_micro++;			/* toggle more micro */
			if (iC_micro >= 3) iC_micro = 0;
#endif	/* YYDEBUG */
		    }
		    /* ignore the rest for now */
		}
	    } else {				/* retval -1 */
		perror("ERROR: select failed");
		iC_quit(SIGUSR1);
	    }
	    /* if many inputs change simultaneously increase oscillator limit */
	    iC_osc_lim = (cnt << 1) + 1;	/* (cnt * 2) + 1 */
	    if (iC_osc_lim < iC_osc_max) {
		iC_osc_lim = iC_osc_max;	/* cnt = 1, osc_lim = 4 default */
	    }
	}
    } /* for (;;) */
} /* iC_icc */

/********************************************************************
 *
 *	Send registration and receive acknowledgement
 *
 *******************************************************************/

static void
regAck(Gate ** oStart, Gate ** oEnd)
{
    Gate **		opp;
    char *		cp;
    Gate *		gp;
    unsigned short	channel;

    if (iC_debug & 04) fprintf(iC_outFP, "register:%s\n", regBuf);
    iC_send_msg_to_server(sockFN, regBuf);	/* register controller and IOs */
    if (iC_rcvd_msg_from_server(sockFN, rpyBuf, REPLY)) {
	if (iC_micro & 06) iC_microPrint("reply from server", 0);
	if (iC_debug & 04) fprintf(iC_outFP, "reply:%s\n", rpyBuf);
	if (Channels == NULL) {
	    // C channel - messages from controller to debugger
	    C_channel = atoi(rpyBuf);
	    assert(C_channel > 0);
	    if (C_channel > topChannel) {
		topChannel = C_channel;
	    }
	    // D channel - messages from debugger to controller
	    cp = strchr(rpyBuf, ',');
	    assert(cp);
	    channel = atoi(++cp);
	    assert(channel > 0);
	    if (channel > topChannel) {
		topChannel = channel;
	    }
	    storeChannel(channel, &D_gate);			/* ==> Debug input */
	    cp = strchr(cp, ',');
	} else {
	    cp = rpyBuf - 1;
	}
	// remaining input and output channel indices matching registrations
	for (opp = oStart; opp < oEnd; opp++) {		/* sTable to sTend */
	    gp = *opp;
	    if ((gp->gt_ini == -INPW && *gp->gt_ids != 'T') ||	/* suppress TX0 */
		gp->gt_fni == OUTW) {
		assert(cp);
		channel = atoi(++cp);
		assert(channel > 0);
		if (channel > topChannel) {
		    topChannel = channel;
		}
		if (gp->gt_ini == -INPW) {
		    storeChannel(channel, gp);			/* ==> Input */
		} else
		if (gp->gt_fni == OUTW) {
		    gp->gt_channel = channel;			/* <== Output */
		}
		cp = strchr(cp, ',');
	    }
	}
	assert(cp == NULL);			/* Ack string matches Registration */
	if (iC_debug & 04) fprintf(iC_outFP, "reply: Channels %hu\n", topChannel);
    } else {
	iC_quit(QUIT_SERVER);			/* quit normally */
    }
} /* regAck */

/********************************************************************
 *
 *	Initalise and expand dynamic array Channels[] as necessary
 *	Store input Gate address in Channels[]
 *
 *******************************************************************/

static void
storeChannel(unsigned short channel, Gate * gp)
{
    while (channel >= ioChannels) {
	Channels = (Gate **)realloc(Channels,	/* initially NULL */
	    (ioChannels + IOCHANNELS) * sizeof(Gate *));
	assert(Channels);
	memset(&Channels[ioChannels], '\0', IOCHANNELS * sizeof(Gate *));
	ioChannels += IOCHANNELS;	/* increase the size of the array */
	if (iC_debug & 04) fprintf(iC_outFP, "storeChannel: Channels[%d] increase\n", ioChannels);
    }
    if (iC_debug & 04) fprintf(iC_outFP, "storeChannel: Channels[%d] <= %s\n", channel, gp->gt_ids);
    Channels[channel] = gp;		/* store input Gate address */
} /* storeChannel */

/********************************************************************
 *
 *	Output VCD data and/or a live data message during scans
 *
 *******************************************************************/

void
#if	INT_MAX == 32767 && defined (LONG16)
iC_liveData(Gate * gp, long value)
#else	/* INT_MAX == 32767 && defined (LONG16) */
iC_liveData(Gate * gp, int value)
#endif	/* INT_MAX == 32767 && defined (LONG16) */
{
    unsigned short	index;
    int			len;
    int			rest;
    char *		code;
    char		binBuf[33];	/* allows 32 bits */

    index = gp->gt_live & 0x7fff;
    if (iC_vcdFP && (code = vcd_ftype[gp->gt_fni]) != NULL) {
	fprintf(iC_vcdFP, "#%ld\n", ++virtualTime);
	switch (*code) {
	case 'e':			/* event */
	case 'w':			/* wire */
#if	INT_MAX == 32767 && defined (LONG16)
	    fprintf(iC_vcdFP, "%1.1ld%hu\n", value, index);
#else	/* INT_MAX == 32767 && defined (LONG16) */
	    fprintf(iC_vcdFP, "%1.1d%hu\n", value, index);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	    break;
	case 'i':			/* integer */
	    convert2binary(binBuf, value, 0);	/* convert to 32 bit binary string */
	    fprintf(iC_vcdFP, "b%s %hu\n", binBuf, index);
	    break;
	default:			/* hard error */
	    assert(0);
	    break;
	}
    }
    if (gp->gt_live & 0x8000) {		/* is variable active for iClive ? */
	while ((len =
#if	INT_MAX == 32767 && defined (LONG16)
		snprintf(&msgBuf[msgOffset], rest = REQUEST - msgOffset,
		    ";%hu %ld", index, value)
#else	/* INT_MAX == 32767 && defined (LONG16) */
		snprintf(&msgBuf[msgOffset], rest = REQUEST - msgOffset,
		    ";%hu %d", index, value)
#endif	/* INT_MAX == 32767 && defined (LONG16) */
		) < 0 || len >= rest
	) {
	    /********************************************************************
	     *  Send live data collected in msgBuf before it overflows to iCserver
	     *******************************************************************/
	    msgBuf[msgOffset] = '\0';		/* terminate */
	    iC_send_msg_to_server(sockFN, msgBuf);
	    msgOffset = liveOffset;			/* msg = "C_channel:3" */
	}
	msgOffset += len;
    }
} /* iC_liveData */

/********************************************************************
 *
 *	Convert an integer to a binary string
 *	binBuf	is a buffer which must be at least 33 characters long
 *	value	is a 32 bit integer
 *	binFlag is either 0, in which up to 32 bits are converted to
 *		a binary string without leading 0's
 *		or it is  1, in which case 8 bits with leading 0's are produced
 *
 *******************************************************************/

static char *
#if	INT_MAX == 32767 && defined (LONG16)
convert2binary(char * binBuf, long value, int binFlag)
#else	/* INT_MAX == 32767 && defined (LONG16) */
convert2binary(char * binBuf, int value, int binFlag)
#endif	/* INT_MAX == 32767 && defined (LONG16) */
{
    char *		binPtr;
#if	INT_MAX == 32767 && defined (LONG16)
    unsigned long	binMask;
#else	/* INT_MAX == 32767 && defined (LONG16) */
    unsigned int	binMask;
#endif	/* INT_MAX == 32767 && defined (LONG16) */
    if (binFlag == 0 && value == 0) {	/* convert to binary string */
	strcpy(binBuf, "0");		/* '0' is the only digit */
    } else {
	binPtr = binBuf;
	binMask = binFlag ? 1 << 7 : 1 << 31;
	do {
	    if (binMask & value) {
		*binPtr++ = '1';	/* start with the left-most '1' digit */
		binFlag = 1;
	    } else if (binFlag) {
		*binPtr++ = '0';
	    }
	} while ((binMask >>= 1) != 0);
	*binPtr++ = '\0';		/* there is at least 1 '1' digit */
    }
    return binBuf;
} /* convert2binary */

/********************************************************************
 *
 *	Initialize IO
 *
 *******************************************************************/

void iC_initIO(void)
{
    signal(SIGSEGV, iC_quit);			/* catch memory access signal */
}

/********************************************************************
 *
 *	Quit program with 'q' or ctrlC or Break via signal SIGINT
 *	or program abort on detected bugs.
 *
 *	Although nanosleep is supposed to be POSIX, it is not implemented
 *	under Windows/Cygwin - since the sleep in iC_quit is not critical
 *	usleep() has been used and not the following, which works under
 *	Linux and MAC-OsX
 *	    struct timespec ms200 = { 0, 200000000, };
 *	    nanosleep(&ms200, NULL);
 *
 *******************************************************************/

void iC_quit(int sig)
{
    /********************************************************************
     *  The following termination function is an empty function
     *  in the libict.a support library.
     *		int iCend(void) { return 0; }
     *  It may be implemented in a literal block of an iC source, in
     *  which case that function will be linked in preference.
     *  User implementations of iCend() should return 1, to activate
     *  the debug message "iend complete ======".
     *
     *  It can be used to free allocated memory, etc.
     *
     *  If the iCbegin() function contains a fork() call, iCend() may have
     *  a matching wait() call.
     *******************************************************************/
    if ((sig >= QUIT_TERMINAL || sig == SIGINT) &&
	iCend()) {				/* termination function */
#if	YYDEBUG
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\niCend complete ======\n");
	}
#endif	/* YYDEBUG */
    }
    if (sockFN) {
	if (C_channel) {
	    /* disconnect iClive - follow with '0' for iCserver */
	    regOffset = snprintf(regBuf, REQUEST, "%hu:5,%hu:0", C_channel, C_channel);
	    iC_send_msg_to_server(sockFN, regBuf);
#ifdef	WIN32
	    Sleep(200);				/* 200 ms in ms */
#else	/* WIN32 */
	    usleep(200000);			/* 200 ms in us */
#endif	/* WIN32 */
	    if (iC_micro) iC_microPrint("Disconnected", 0);
	}
	close(sockFN);				/* close connection to iCserver */
    }
    if (sig == QUIT_TERMINAL) {
	fprintf(iC_errFP, "\n'%s' stopped from terminal\n", iC_iccNM);
    } else if (sig == QUIT_DEBUGGER) {
	fprintf(iC_errFP, "\n'%s' stopped by debugger\n", iC_iccNM);
    } else if (sig == QUIT_SERVER) {
	fprintf(iC_errFP, "\n'%s' disconnected by server\n", iC_iccNM);
    } else if (sig == SIGINT) {
	fprintf(iC_errFP, "\n'%s' stopped by interrupt from terminal\n", iC_iccNM);
    } else if (sig == SIGSEGV) {
	fprintf(iC_errFP, "\n'%s' stopped by 'Invalid memory reference'\n", iC_iccNM);
    } else if (sig == SIGUSR1) {
	fprintf(iC_errFP, "\n'%s' stopped by 'non-recoverable run-time error'\n", iC_iccNM);
    }
    if (iC_savFP) {
	fflush(iC_savFP);
	fclose(iC_savFP);
    }
    if (iC_vcdFP) {
	fflush(iC_vcdFP);
	fclose(iC_vcdFP);
    }
    if (iC_outFP != stdout) {
	fflush(iC_outFP);
	fclose(iC_outFP);
    }
    if (iC_errFP != stderr) {
	fflush(iC_errFP);
	fclose(iC_errFP);
    }
#ifdef	EFENCE
    free(rpyBuf);
    free(iC_outBuf);
    free(regBuf);
    free(msgBuf);
#endif	/* EFENCE */
    exit(sig < QUIT_TERMINAL ? sig : 0);	/* really quit */
} /* iC_quit */
#endif	/* TCP */

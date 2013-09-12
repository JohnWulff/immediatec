static const char init_c[] =
"@(#)$Id: init.c,v 1.40 2013/08/12 08:08:47 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2009  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	init.c
 *	immediate C - initialise compiler symbol table with keywords
 *
 *******************************************************************/

#ifdef _WINDOWS
#include	<windows.h>
#endif
#include	<stdio.h>
#include	<assert.h>
#include	<string.h>
#include	"icc.h"
#include	"comp.h"
#include	"comp_tab.h"

static BuiltIn b[];

Symbol *	iclock;			/* default clock */
Symbol *	iconst;			/* pointer to Symbol "iConst" */
Symbol *	icerr = 0;		/* pointer to Symbol "iCerr" */

const char *	genLines[]  = {
    "",
    "#line 202 \"init.c\"	/* in pre-compiled function block SHR */",
    "#line 204 \"init.c\"	/* in pre-compiled function block SHSR */",
};	/* last entry must have index [GEN_COUNT] */

const char *	genName = "init.c";	/* must be the same name as above */

int		genLineNums[] = {
    0,
    202,				/* must be the same lines as above */
    204,
};	/* last entry must have index [GEN_COUNT] */

/********************************************************************
 *
 *	Install reserved words and pre-compiled built-ins
 *	or yacc tokens for old style built-ins if BOOT_COMPILE defined
 *
 *******************************************************************/

void
init(void)
{
    int		io;
    int		num;
    Symbol *	sp;
    Symbol *	tsp;
    List_e *	lp;
    FILE *	H1p;
    char	lineBuf[BUFS];
    char	tag[BUFS];
    char	ids[BUFS];

    assert(sizeof genLines/sizeof genLines[0] - 1 == GEN_COUNT);
    assert(sizeof genLineNums/sizeof genLineNums[0] - 1 == GEN_COUNT);

    if ((iC_debug & 010000) && inpFN == 0) {
	fprintf(iC_outFP, "initialFunctions:\n%s\n", initialFunctions);
    }
    for (io = 0; b[io].name; io++) {
	/* no name may occurr twice - otherwise install fails */
	iconst = sp;	/* 'iconst' points to "iConst" when loop finishes */
	sp = install(b[io].name, b[io].type, b[io].ftype);
	if (b[io].type == KEYW) {		/* reserved word */
	    sp->u_val = b[io].uVal;		/* set u_val in Symbol just installed */
	} else {
	    sp->fm = b[io].uVal;		/* set fm in Symbol just installed */
	    sp->list = b[io].list;		/* set list in Symbol just installed */
	    sp->u_blist = b[io].blist;		/* set u_blist in Symbol just installed */
	    if ((lp = b[io].link1) != 0) {
		lp->le_sym = sp;		/* 1st back link to complete built-in net */
	    }
	    if ((lp = b[io].link2) != 0) {
		if (sp->type == IFUNCT) {
		    tsp = lookup(((BuiltIn*)lp)->name);
		    assert(tsp->v_glist == 0);
		    tsp->v_glist = sp;		/* forward Symbol link to alternate IFUNCT */
		} else {
		    lp->le_sym = sp;		/* 2nd back link to complete built-in net */
		}
	    }
	}
    }
    iclock = sp;	/* 'iclock' with name "iClock" is default clock when loop finishes */

    if (iC_aflag != 0) {		/* build several files to be linked */
	if ((H1p = fopen(H1name, "r")) != 0) {			/* scan .iC_list1.h */
	    while (fgets(lineBuf, sizeof lineBuf, H1p)) {	/* read previous declarations */
		if (sscanf(lineBuf, "/* %s\t%s\t%d */", tag, ids, &num) == 3) {
		    if (strcmp(tag, "TIMER1:") == 0) {
			if ((sp = lookup(ids)) == 0) {
			    sp = install(ids, TIM, TIMRL);	/* TIMER1 not declared yet */
			    sp->em |= EM;			/* initially extern */
			}
			sp->em |= num;				/* TM1 if TIMER1 */
		    } else if (strcmp(tag, "InstanceNum:") == 0) {
			if ((sp = lookup(ids)) == 0) {
			    sp = install(ids, IFUNCT, UDFA);	/* function not defined yet */
			}
			/* care is taken that union members v.elist and v.glist not used for IFUNCT */
			assert(sp->type == IFUNCT && sp->v_cnt == 0);
			sp->v_cnt = num;			/* initialise instance number */
		    }
		}
	    }
	    fclose(H1p);
	} else {
	    iC_lflag = 1;			/* need to write #define in new .iC_list1.h */
	}
    }
} /* init */

/********************************************************************
 *
 *	These definitions are parsed before any line of iC code is read.
 *
 *	WARNING: make sure lines are not more than CBUFSZ-1 (124) chars long
 *		 otherwise sytem aborts in get().
 *
 *	Suggestion: make internally declared immediate variables 1 char long
 *		 to keep generated listing names short. 'i' is a good choice.
 *
 *	A listing of these functions can be generated with    -d10044
 *
 *  The first two function blocks generate C functions 1 and 2
 *  *** adjust line numbers to point to correct line in initialFunctions
 *  *** comment string is required in comp.y at /has comment string after/
 *  *** adjust GEN_COUNT in comp.h to reflect correct number (used in icc.c)
 *
 *  genName and genLineNums are used to identify function lines in get()
 *  generated with genLines.
 *
 *******************************************************************/

/********************************************************************
 *
 *  Cross checked SR is not the default because of extra code generated
 *  which is normally not needed - SRX is provided if it is required
 *
 *  The C keywords cause a syntax error if used as immediate variables.
 *  The erroneous line in the iC code is marked with a pointer to the
 *  offending keyword.
 *  Otherwise the C compiler would flag them with a syntax error message
 *  referring to the line number of the generated Gate in the C code
 *  produced by th iC compiler, which is harder to trace back to the source.
 *  C keywords int if else extern return switch and void which are keywords
 *  in iC as well cause syntax errors also if used as immediate variables.
 *
 *******************************************************************/

/********************************************************************
 *  Trial version with all int parameters - useful with ARITHMETIC optimisation.
 *
imm int SHR(int dat, clock dcl, int res, clock rcl) {\n\
    this = SHR_( res ? this : dat, dcl, res, rcl); }	// line 202\n\
imm int SHSR(int dat, clock dcl, int set, clock scl, int res, clock rcl) {\n\
    this = SHSR_(set || res ? this : dat, dcl, set && !res, scl, res && !set, rcl); }\n\
 *
 *  alternate lines in genLines[]
    "#line 202 \"init.c\"	// 'dat' in pre-compiled function block SHR",
    "#line 204 \"init.c\"	// 'dat' in pre-compiled function block SHSR",
    "#line 204 \"init.c\"	// 'set' in pre-compiled function block SHSR",
    "#line 204 \"init.c\"	// 'res' in pre-compiled function block SHSR",
 *  extra middle lines in genLineNums[]
    204,
    204,
 *  Integration for pre-compile code with more than 2 expressions has been done
 *  This code has been tested with #define GEN_COUNT 4 - jw 070316
 *******************************************************************/

/* iC system function definitions: keep SHR() at line 202, SHSR at line 204 */
const char initialFunctions[] = "\
/* iC system function definitions */\n\
imm bit SR(bit set, clock scl, bit res, clock rcl) {\n\
    this = SR_(set, scl, res, rcl); }\n\
imm bit SRX(bit set, clock scl, bit res, clock rcl) {\n\
    this = SR_(set & ~res, scl, ~set & res, rcl); }\n\
imm bit JK(bit set, clock scl, bit res, clock rcl) {\n\
    this = SR_(~this & set, scl, this & res, rcl); }\n\
imm bit SRR(bit set, clock scl, bit res, clock rcl, bit res2, clock r2cl) {\n\
    this = SRR_(set, scl, res, rcl, res2, r2cl); }\n\
imm bit DR(bit dat, clock dcl, bit res, clock rcl) {\n\
    this = DR_(dat & ~res | this & res, dcl, res, rcl); }\n\
imm bit DSR(bit dat, clock dcl, bit set, clock scl, bit res, clock rcl) {\n\
    imm bit i = set | res;\n\
    this = DSR_(dat & ~i | this & i, dcl, set & ~res, scl, ~set & res, rcl); }\n\
imm int SHR(int dat, clock dcl, bit res, clock rcl) {\n\
    this = SHR_( res ? this : dat, dcl, res, rcl); }	/* line 202 */\n\
imm int SHSR(int dat, clock dcl, bit set, clock scl, bit res, clock rcl) {\n\
    this = SHSR_(set | res ? this : dat, dcl, set & ~res, scl, ~set & res, rcl); }\n\
imm bit ST(bit set, clock scl, clock tcl) {\n\
    this = SR_(set, scl, this, tcl); }\n\
imm bit SRT(bit set, clock scl, bit res, clock rcl, clock tcl) {\n\
    this = SRR_(set, scl, res, rcl, this, tcl); }\n\
imm bit FALL(bit fall, clock clk) {\n\
    this = RISE(~fall, clk); }\n\
imm bit LATCH(bit set, bit res) {\n\
    this = FORCE(this, set, res); }\n\
imm bit DLATCH(bit set, bit res, clock clk) {\n\
    this = D(FORCE(this, set, res), clk); }\n\
/* end iC system function definitions */\n\
";

#ifdef BOOT_COMPILE
Symbol		iC_CHANGE_ar = { "CHANGE", KEYW, CH_AR, };	/* alternative arithmetic CHANGE */
#else	/* BOOT_COMPILE */
/********************************************************************
 * Pre-compiled built-in functions produced in functionDefinition()
 *
 * Bootstrapping the original built-in functions, requires an older
 * version of the compiler which compiles built-ins directly.
 *
 * if BOOT_COMPILE is defined this direct compilation is left active
 * and any pre-compiled functions are left out of init.c.
 *******************************************************************/

static List_e	l[];

static Symbol	Da_1       = { .type=OR , .ftype=D_FF  , .fm=0x01, .u_blist=&l[13] };

static Symbol	SR_a_2     = { .type=OR , .ftype=R_FF  , .fm=0x01, .u_blist=&l[24] };
static Symbol	SR_a_1     = { .type=OR , .ftype=S_FF  , .fm=0x01, .u_blist=&l[22], .next=&SR_a_2 };

static Symbol	DR_a_2     = { .type=OR , .ftype=R_FF  , .fm=0x01, .u_blist=&l[37] };
static Symbol	DR_a_1     = { .type=OR , .ftype=D_FF  , .fm=0x01, .u_blist=&l[35], .next=&DR_a_2 };

static Symbol	SRR_a_3    = { .type=OR , .ftype=R_FF  , .fm=0x01, .u_blist=&l[53] };
static Symbol	SRR_a_2    = { .type=OR , .ftype=R_FF  , .fm=0x01, .u_blist=&l[51], .next=&SRR_a_3 };
static Symbol	SRR_a_1    = { .type=OR , .ftype=S_FF  , .fm=0x01, .u_blist=&l[49], .next=&SRR_a_2 };

static Symbol	DSR_a_3    = { .type=OR , .ftype=R_FF  , .fm=0x01, .u_blist=&l[71] };
static Symbol	DSR_a_2    = { .type=OR , .ftype=S_FF  , .fm=0x01, .u_blist=&l[69], .next=&DSR_a_3 };
static Symbol	DSR_a_1    = { .type=OR , .ftype=D_FF  , .fm=0x01, .u_blist=&l[67], .next=&DSR_a_2 };

static Symbol	SHa_1      = { .type=ARN, .ftype=D_SH  , .fm=0x01, .u_blist=&l[83] };

static Symbol	SHR_a_2    = { .type=OR , .ftype=R_SH  , .fm=0x01, .u_blist=&l[94] };
static Symbol	SHR_a_1    = { .type=ARN, .ftype=D_SH  , .fm=0x01, .u_blist=&l[92], .next=&SHR_a_2 };

static Symbol	SHSR_a_3   = { .type=OR , .ftype=R_SH  , .fm=0x01, .u_blist=&l[110] };
static Symbol	SHSR_a_2   = { .type=OR , .ftype=S_SH  , .fm=0x01, .u_blist=&l[108], .next=&SHSR_a_3 };
static Symbol	SHSR_a_1   = { .type=ARN, .ftype=D_SH  , .fm=0x01, .u_blist=&l[106], .next=&SHSR_a_2 };

static Symbol	RISEa_1    = { .type=OR , .ftype=RI_BIT, .fm=0x01, .u_blist=&l[122] };

static Symbol	CHANGEa_1  = { .type=OR , .ftype=CH_BIT, .fm=0x01, .u_blist=&l[130] };

static Symbol	CHANGE2a_1 = { .type=ARN, .ftype=CH_AR , .fm=0x01, .u_blist=&l[138] };

static Symbol	CLOCKa_1   = { .type=OR , .ftype=CLCK  , .fm=0x01, .u_blist=&l[146] };

static Symbol	CLOCK2a_2  = { .type=OR , .ftype=CLCK  , .fm=0x01, .u_blist=&l[157] };
static Symbol	CLOCK2a_1  = { .type=OR , .ftype=CLCK  , .fm=0x01, .u_blist=&l[155], .next=&CLOCK2a_2 };

static Symbol	TIMERa_1   = { .type=OR , .ftype=TIMR  , .fm=0x01, .u_blist=&l[167] };

static Symbol	TIMER2a_2  = { .type=OR , .ftype=TIMR  , .fm=0x01, .u_blist=&l[178] };
static Symbol	TIMER2a_1  = { .type=OR , .ftype=TIMR  , .fm=0x01, .u_blist=&l[176], .next=&TIMER2a_2 };

static Symbol	TIMER1a_1  = { .type=OR , .ftype=TIMR  , .fm=0x01, .u_blist=&l[188] };

static Symbol	TIMER12a_2 = { .type=OR , .ftype=TIMR  , .fm=0x01, .u_blist=&l[199] };
static Symbol	TIMER12a_1 = { .type=OR , .ftype=TIMR  , .fm=0x01, .u_blist=&l[197], .next=&TIMER12a_2 };

static List_e	l[] = {
/*         le_sym    le_val  le_next  le_first le_last */

/*   0 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  FORCE */
/*   1 */ { 0          ,  0, &l[2]  , 0 , 0 , },	/* ==>  FORCE@ */
/*   2 */ { 0          ,  3, 0      , 0 , 0 , },	/* ==>  no var list Symbols */
/*   3 */ { 0          ,  0, &l[4]  , "", "", },	/* ==>  FORCE@arg1 */
/*   4 */ { 0          ,  0, &l[5]  , "", "", },	/* ==>  FORCE@on */
/*   5 */ { 0          ,  1, 0      , "", "", },	/* ==> ~FORCE@off */
/*   6 */ { 0          ,  0, &l[7]  , "", "", },	/* ==>  FORCE@arg1 */
/*   7 */ { 0          ,  0, &l[8]  , "", "", },	/* ==>  FORCE@on */
/*   8 */ { 0          ,  0, 0      , "", "", },	/* ==>  FORCE@off */

/*   9 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  D */
/*  10 */ { 0          ,  0, &l[11] , 0 , 0 , },	/* ==>  D@ */
/*  11 */ { &Da_1      ,  2, 0      , 0 , 0 , },	/* ==>  Da_1 */
/*  12 */ { &Da_1      ,  0, 0      , "", "", },	/* ==>  Da_1 */
/*  13 */ { 0          ,  0, &l[14] , 0 , 0 , },	/* ==>  D@dcl */
/*  14 */ { 0          ,  0, 0      , "", "", },	/* ==>  D@expr */
/*  15 */ { 0          ,  0, &l[16] , "", "", },	/* ==>  D@expr */
/*  16 */ { 0          ,  0, 0      , "", "", },	/* ==>  D@dcl */

/*  17 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  SR_ */
/*  18 */ { 0          ,  0, &l[19] , 0 , 0 , },	/* ==>  SR_@ */
/*  19 */ { &SR_a_1    ,  4, 0      , 0 , 0 , },	/* ==>  SR_a_1 */
/*  20 */ { &SR_a_1    ,  0, &l[21] , "", "", },	/* ==>  SR_a_1 */
/*  21 */ { &SR_a_2    ,  0, 0      , "", "", },	/* ==>  SR_a_2 */
/*  22 */ { 0          ,  0, &l[23] , 0 , 0 , },	/* ==>  SR_@scl */
/*  23 */ { 0          ,  0, 0      , "", "", },	/* ==>  SR_@set */
/*  24 */ { 0          ,  0, &l[25] , 0 , 0 , },	/* ==>  SR_@rcl */
/*  25 */ { 0          ,  0, 0      , "", "", },	/* ==>  SR_@res */
/*  26 */ { 0          ,  0, &l[27] , "", "", },	/* ==>  SR_@set */
/*  27 */ { 0          ,  0, &l[28] , "", "", },	/* ==>  SR_@scl */
/*  28 */ { 0          ,  0, &l[29] , "", "", },	/* ==>  SR_@res */
/*  29 */ { 0          ,  0, 0      , "", "", },	/* ==>  SR_@rcl */

/*  30 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  DR_ */
/*  31 */ { 0          ,  0, &l[32] , 0 , 0 , },	/* ==>  DR_@ */
/*  32 */ { &DR_a_1    ,  4, 0      , 0 , 0 , },	/* ==>  DR_a_1 */
/*  33 */ { &DR_a_1    ,  0, &l[34] , "", "", },	/* ==>  DR_a_1 */
/*  34 */ { &DR_a_2    ,  0, 0      , "", "", },	/* ==>  DR_a_2 */
/*  35 */ { 0          ,  0, &l[36] , 0 , 0 , },	/* ==>  DR_@dcl */
/*  36 */ { 0          ,  0, 0      , "", "", },	/* ==>  DR_@expr */
/*  37 */ { 0          ,  0, &l[38] , 0 , 0 , },	/* ==>  DR_@rcl */
/*  38 */ { 0          ,  0, 0      , "", "", },	/* ==>  DR_@res */
/*  39 */ { 0          ,  0, &l[40] , "", "", },	/* ==>  DR_@expr */
/*  40 */ { 0          ,  0, &l[41] , "", "", },	/* ==>  DR_@dcl */
/*  41 */ { 0          ,  0, &l[42] , "", "", },	/* ==>  DR_@res */
/*  42 */ { 0          ,  0, 0      , "", "", },	/* ==>  DR_@rcl */

/*  43 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  SRR_ */
/*  44 */ { 0          ,  0, &l[45] , 0 , 0 , },	/* ==>  SRR_@ */
/*  45 */ { &SRR_a_1   ,  6, 0      , 0 , 0 , },	/* ==>  SRR_a_1 */
/*  46 */ { &SRR_a_1   ,  0, &l[47] , "", "", },	/* ==>  SRR_a_1 */
/*  47 */ { &SRR_a_2   ,  0, &l[48] , "", "", },	/* ==>  SRR_a_2 */
/*  48 */ { &SRR_a_3   ,  0, 0      , "", "", },	/* ==>  SRR_a_3 */
/*  49 */ { 0          ,  0, &l[50] , 0 , 0 , },	/* ==>  SRR_@scl */
/*  50 */ { 0          ,  0, 0      , "", "", },	/* ==>  SRR_@set */
/*  51 */ { 0          ,  0, &l[52] , 0 , 0 , },	/* ==>  SRR_@rcl */
/*  52 */ { 0          ,  0, 0      , "", "", },	/* ==>  SRR_@res */
/*  53 */ { 0          ,  0, &l[54] , 0 , 0 , },	/* ==>  SRR_@r2cl */
/*  54 */ { 0          ,  0, 0      , "", "", },	/* ==>  SRR_@res2 */
/*  55 */ { 0          ,  0, &l[56] , "", "", },	/* ==>  SRR_@set */
/*  56 */ { 0          ,  0, &l[57] , "", "", },	/* ==>  SRR_@scl */
/*  57 */ { 0          ,  0, &l[58] , "", "", },	/* ==>  SRR_@res */
/*  58 */ { 0          ,  0, &l[59] , "", "", },	/* ==>  SRR_@rcl */
/*  59 */ { 0          ,  0, &l[60] , "", "", },	/* ==>  SRR_@res2 */
/*  60 */ { 0          ,  0, 0      , "", "", },	/* ==>  SRR_@r2cl */

/*  61 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  DSR_ */
/*  62 */ { 0          ,  0, &l[63] , 0 , 0 , },	/* ==>  DSR_@ */
/*  63 */ { &DSR_a_1   ,  6, 0      , 0 , 0 , },	/* ==>  DSR_a_1 */
/*  64 */ { &DSR_a_1   ,  0, &l[65] , "", "", },	/* ==>  DSR_a_1 */
/*  65 */ { &DSR_a_2   ,  0, &l[66] , "", "", },	/* ==>  DSR_a_2 */
/*  66 */ { &DSR_a_3   ,  0, 0      , "", "", },	/* ==>  DSR_a_3 */
/*  67 */ { 0          ,  0, &l[68] , 0 , 0 , },	/* ==>  DSR_@dcl */
/*  68 */ { 0          ,  0, 0      , "", "", },	/* ==>  DSR_@expr */
/*  69 */ { 0          ,  0, &l[70] , 0 , 0 , },	/* ==>  DSR_@scl */
/*  70 */ { 0          ,  0, 0      , "", "", },	/* ==>  DSR_@set */
/*  71 */ { 0          ,  0, &l[72] , 0 , 0 , },	/* ==>  DSR_@rcl */
/*  72 */ { 0          ,  0, 0      , "", "", },	/* ==>  DSR_@res */
/*  73 */ { 0          ,  0, &l[74] , "", "", },	/* ==>  DSR_@expr */
/*  74 */ { 0          ,  0, &l[75] , "", "", },	/* ==>  DSR_@dcl */
/*  75 */ { 0          ,  0, &l[76] , "", "", },	/* ==>  DSR_@set */
/*  76 */ { 0          ,  0, &l[77] , "", "", },	/* ==>  DSR_@scl */
/*  77 */ { 0          ,  0, &l[78] , "", "", },	/* ==>  DSR_@res */
/*  78 */ { 0          ,  0, 0      , "", "", },	/* ==>  DSR_@rcl */

/*  79 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  SH */
/*  80 */ { 0          ,  0, &l[81] , 0 , 0 , },	/* ==>  SH@ */
/*  81 */ { &SHa_1     ,  2, 0      , 0 , 0 , },	/* ==>  SHa_1 */
/*  82 */ { &SHa_1     ,  0, 0      , "", "", },	/* ==>  SHa_1 */
/*  83 */ { 0          ,  0, &l[84] , 0 , 0 , },	/* ==>  SH@dcl */
/*  84 */ { 0          ,  0, 0      , "", "", },	/* ==>  SH@dat */
/*  85 */ { 0          ,  0, &l[86] , "", "", },	/* ==>  SH@dat */
/*  86 */ { 0          ,  0, 0      , "", "", },	/* ==>  SH@dcl */

/*  87 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  SHR_ */
/*  88 */ { 0          ,  0, &l[89] , 0 , 0 , },	/* ==>  SHR_@ */
/*  89 */ { &SHR_a_1   ,  4, 0      , 0 , 0 , },	/* ==>  SHR_a_1 */
/*  90 */ { &SHR_a_1   ,  0, &l[91] , "", "", },	/* ==>  SHR_a_1 */
/*  91 */ { &SHR_a_2   ,  0, 0      , "", "", },	/* ==>  SHR_a_2 */
/*  92 */ { 0          ,  0, &l[93] , 0 , 0 , },	/* ==>  SHR_@dcl */
/*  93 */ { 0          ,  0, 0      , "", "", },	/* ==>  SHR_@dat */
/*  94 */ { 0          ,  0, &l[95] , 0 , 0 , },	/* ==>  SHR_@rcl */
/*  95 */ { 0          ,  0, 0      , "", "", },	/* ==>  SHR_@res */
/*  96 */ { 0          ,  0, &l[97] , "", "", },	/* ==>  SHR_@dat */
/*  97 */ { 0          ,  0, &l[98] , "", "", },	/* ==>  SHR_@dcl */
/*  98 */ { 0          ,  0, &l[99] , "", "", },	/* ==>  SHR_@res */
/*  99 */ { 0          ,  0, 0      , "", "", },	/* ==>  SHR_@rcl */

/* 100 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  SHSR_ */
/* 101 */ { 0          ,  0, &l[102], 0 , 0 , },	/* ==>  SHSR_@ */
/* 102 */ { &SHSR_a_1  ,  6, 0      , 0 , 0 , },	/* ==>  SHSR_a_1 */
/* 103 */ { &SHSR_a_1  ,  0, &l[104], "", "", },	/* ==>  SHSR_a_1 */
/* 104 */ { &SHSR_a_2  ,  0, &l[105], "", "", },	/* ==>  SHSR_a_2 */
/* 105 */ { &SHSR_a_3  ,  0, 0      , "", "", },	/* ==>  SHSR_a_3 */
/* 106 */ { 0          ,  0, &l[107], 0 , 0 , },	/* ==>  SHSR_@dcl */
/* 107 */ { 0          ,  0, 0      , "", "", },	/* ==>  SHSR_@dat */
/* 108 */ { 0          ,  0, &l[109], 0 , 0 , },	/* ==>  SHSR_@scl */
/* 109 */ { 0          ,  0, 0      , "", "", },	/* ==>  SHSR_@set */
/* 110 */ { 0          ,  0, &l[111], 0 , 0 , },	/* ==>  SHSR_@rcl */
/* 111 */ { 0          ,  0, 0      , "", "", },	/* ==>  SHSR_@res */
/* 112 */ { 0          ,  0, &l[113], "", "", },	/* ==>  SHSR_@dat */
/* 113 */ { 0          ,  0, &l[114], "", "", },	/* ==>  SHSR_@dcl */
/* 114 */ { 0          ,  0, &l[115], "", "", },	/* ==>  SHSR_@set */
/* 115 */ { 0          ,  0, &l[116], "", "", },	/* ==>  SHSR_@scl */
/* 116 */ { 0          ,  0, &l[117], "", "", },	/* ==>  SHSR_@res */
/* 117 */ { 0          ,  0, 0      , "", "", },	/* ==>  SHSR_@rcl */

/* 118 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  RISE */
/* 119 */ { 0          ,  0, &l[120], 0 , 0 , },	/* ==>  RISE@ */
/* 120 */ { &RISEa_1   ,  2, 0      , 0 , 0 , },	/* ==>  RISEa_1 */
/* 121 */ { &RISEa_1   ,  0, 0      , "", "", },	/* ==>  RISEa_1 */
/* 122 */ { 0          ,  0, &l[123], 0 , 0 , },	/* ==>  RISE@clk */
/* 123 */ { 0          ,  0, 0      , "", "", },	/* ==>  RISE@trig */
/* 124 */ { 0          ,  0, &l[125], "", "", },	/* ==>  RISE@trig */
/* 125 */ { 0          ,  0, 0      , "", "", },	/* ==>  RISE@clk */

/* 126 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  CHANGE */
/* 127 */ { 0          ,  0, &l[128], 0 , 0 , },	/* ==>  CHANGE@ */
/* 128 */ { &CHANGEa_1 ,  2, 0      , 0 , 0 , },	/* ==>  CHANGEa_1 */
/* 129 */ { &CHANGEa_1 ,  0, 0      , "", "", },	/* ==>  CHANGEa_1 */
/* 130 */ { 0          ,  0, &l[131], 0 , 0 , },	/* ==>  CHANGE@clk */
/* 131 */ { 0          ,  0, 0      , "", "", },	/* ==>  CHANGE@trig */
/* 132 */ { 0          ,  0, &l[133], "", "", },	/* ==>  CHANGE@trig */
/* 133 */ { 0          ,  0, 0      , "", "", },	/* ==>  CHANGE@clk */

/* 134 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  CHANGE2 */
/* 135 */ { 0          ,  0, &l[136], 0 , 0 , },	/* ==>  CHANGE2@ */
/* 136 */ { &CHANGE2a_1,  2, 0      , 0 , 0 , },	/* ==>  CHANGE2a_1 */
/* 137 */ { &CHANGE2a_1,  0, 0      , "", "", },	/* ==>  CHANGE2a_1 */
/* 138 */ { 0          ,  0, &l[139], 0 , 0 , },	/* ==>  CHANGE2@clk */
/* 139 */ { 0          ,  0, 0      , "", "", },	/* ==>  CHANGE2@val */
/* 140 */ { 0          ,  0, &l[141], "", "", },	/* ==>  CHANGE2@val */
/* 141 */ { 0          ,  0, 0      , "", "", },	/* ==>  CHANGE2@clk */

/* 142 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  CLOCK */
/* 143 */ { 0          ,  0, &l[144], 0 , 0 , },	/* ==>  CLOCK@ */
/* 144 */ { &CLOCKa_1  ,  2, 0      , 0 , 0 , },	/* ==>  CLOCKa_1 */
/* 145 */ { &CLOCKa_1  ,  0, 0      , "", "", },	/* ==>  CLOCKa_1 */
/* 146 */ { 0          ,  0, &l[147], 0 , 0 , },	/* ==>  CLOCK@clk */
/* 147 */ { 0          ,  0, 0      , "", "", },	/* ==>  CLOCK@trig */
/* 148 */ { 0          ,  0, &l[149], "", "", },	/* ==>  CLOCK@trig */
/* 149 */ { 0          ,  0, 0      , "", "", },	/* ==>  CLOCK@clk */

/* 150 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  CLOCK2 */
/* 151 */ { 0          ,  0, &l[152], 0 , 0 , },	/* ==>  CLOCK2@ */
/* 152 */ { &CLOCK2a_1 ,  4, 0      , 0 , 0 , },	/* ==>  CLOCK2a_1 */
/* 153 */ { &CLOCK2a_1 ,  0, &l[154], "", "", },	/* ==>  CLOCK2a_1 */
/* 154 */ { &CLOCK2a_2 ,  0, 0      , "", "", },	/* ==>  CLOCK2a_2 */
/* 155 */ { 0          ,  0, &l[156], 0 , 0 , },	/* ==>  CLOCK2@clk */
/* 156 */ { 0          ,  0, 0      , "", "", },	/* ==>  CLOCK2@trig */
/* 157 */ { 0          ,  0, &l[158], 0 , 0 , },	/* ==>  CLOCK2@clk2 */
/* 158 */ { 0          ,  0, 0      , "", "", },	/* ==>  CLOCK2@trig2 */
/* 159 */ { 0          ,  0, &l[160], "", "", },	/* ==>  CLOCK2@trig */
/* 160 */ { 0          ,  0, &l[161], "", "", },	/* ==>  CLOCK2@clk */
/* 161 */ { 0          ,  0, &l[162], "", "", },	/* ==>  CLOCK2@trig2 */
/* 162 */ { 0          ,  0, 0      , "", "", },	/* ==>  CLOCK2@clk2 */

/* 163 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  TIMER */
/* 164 */ { 0          ,  0, &l[165], 0 , 0 , },	/* ==>  TIMER@ */
/* 165 */ { &TIMERa_1  ,  2, 0      , 0 , 0 , },	/* ==>  TIMERa_1 */
/* 166 */ { &TIMERa_1  ,  0, 0      , "", "", },	/* ==>  TIMERa_1 */
/* 167 */ { 0          ,  0, &l[168], 0 , 0 , },	/* ==>  TIMER@clk */
/* 168 */ { 0          ,  0, 0      , "", "", },	/* ==>  TIMER@trig */
/* 169 */ { 0          ,  0, &l[170], "", "", },	/* ==>  TIMER@trig */
/* 170 */ { 0          ,  0, 0      , "", "", },	/* ==>  TIMER@clk */

/* 171 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  TIMER2 */
/* 172 */ { 0          ,  0, &l[173], 0 , 0 , },	/* ==>  TIMER2@ */
/* 173 */ { &TIMER2a_1 ,  4, 0      , 0 , 0 , },	/* ==>  TIMER2a_1 */
/* 174 */ { &TIMER2a_1 ,  0, &l[175], "", "", },	/* ==>  TIMER2a_1 */
/* 175 */ { &TIMER2a_2 ,  0, 0      , "", "", },	/* ==>  TIMER2a_2 */
/* 176 */ { 0          ,  0, &l[177], 0 , 0 , },	/* ==>  TIMER2@clk */
/* 177 */ { 0          ,  0, 0      , "", "", },	/* ==>  TIMER2@trig */
/* 178 */ { 0          ,  0, &l[179], 0 , 0 , },	/* ==>  TIMER2@clk2 */
/* 179 */ { 0          ,  0, 0      , "", "", },	/* ==>  TIMER2@trig2 */
/* 180 */ { 0          ,  0, &l[181], "", "", },	/* ==>  TIMER2@trig */
/* 181 */ { 0          ,  0, &l[182], "", "", },	/* ==>  TIMER2@clk */
/* 182 */ { 0          ,  0, &l[183], "", "", },	/* ==>  TIMER2@trig2 */
/* 183 */ { 0          ,  0, 0      , "", "", },	/* ==>  TIMER2@clk2 */

/* 184 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  TIMER1 */
/* 185 */ { 0          ,  0, &l[186], 0 , 0 , },	/* ==>  TIMER1@ */
/* 186 */ { &TIMER1a_1 ,  2, 0      , 0 , 0 , },	/* ==>  TIMER1a_1 */
/* 187 */ { &TIMER1a_1 ,  1, 0      , "", "", },	/* ==> ~TIMER1a_1 */
/* 188 */ { 0          ,  0, &l[189], 0 , 0 , },	/* ==>  TIMER1@clk */
/* 189 */ { 0          ,  0, 0      , "", "", },	/* ==>  TIMER1@trig */
/* 190 */ { 0          ,  0, &l[191], "", "", },	/* ==>  TIMER1@trig */
/* 191 */ { 0          ,  0, 0      , "", "", },	/* ==>  TIMER1@clk */

/* 192 */ { 0          ,  0, 0      , 0 , 0 , },	/* ==>  TIMER12 */
/* 193 */ { 0          ,  0, &l[194], 0 , 0 , },	/* ==>  TIMER12@ */
/* 194 */ { &TIMER12a_1,  4, 0      , 0 , 0 , },	/* ==>  TIMER12a_1 */
/* 195 */ { &TIMER12a_1,  1, &l[196], "", "", },	/* ==> ~TIMER12a_1 */
/* 196 */ { &TIMER12a_2,  0, 0      , "", "", },	/* ==>  TIMER12a_2 */
/* 197 */ { 0          ,  0, &l[198], 0 , 0 , },	/* ==>  TIMER12@clk */
/* 198 */ { 0          ,  0, 0      , "", "", },	/* ==>  TIMER12@trig */
/* 199 */ { 0          ,  0, &l[200], 0 , 0 , },	/* ==>  TIMER12@clk2 */
/* 200 */ { 0          ,  0, 0      , "", "", },	/* ==>  TIMER12@trig2 */
/* 201 */ { 0          ,  0, &l[202], "", "", },	/* ==>  TIMER12@trig */
/* 202 */ { 0          ,  0, &l[203], "", "", },	/* ==>  TIMER12@clk */
/* 203 */ { 0          ,  0, &l[204], "", "", },	/* ==>  TIMER12@trig2 */
/* 204 */ { 0          ,  0, 0      , "", "", },	/* ==>  TIMER12@clk2 */
};

#endif	/* BOOT_COMPILE */
/********************************************************************
 *
 *	Initialise Symbol table with all reserved words for iC and C
 *
 *		built-in symbols:	"name"	for built-in
 *					type	for built-in
 *					ftype	for built-in
 *					fm	usage for built-in Symbol
 *					list	output list pointer
 *					blist	forward blist pointer
 *					link1	2 back links for new built-in
 *					link2	set up in BOOT_COMPILE
 *
 *		reserved words:		"name"
 *					type	KEYW etc
 *					ftype	used in compilation
 *					uVal	compiler_token
 *
 *******************************************************************/

static BuiltIn b[] = {
#ifndef BOOT_COMPILE
  /* name            type   ftype  fm    list     blist    link1    link2/glist */
  { "FORCE",       IFUNCT,  GATE,  0x3f, &l[6]  , &l[1]  , &l[0]  , 0      , }, /* 0 pre-installed FORCE function block */
  { "FORCE@",        LATCH, GATE,  0x80, &l[0]  , &l[3]  , &l[1]  , 0      , },
  { "FORCE@arg1",    OR,    GATE,  0x81, 0      , 0      , &l[3]  , &l[6]  , },
  { "FORCE@on",      OR,    GATE,  0x81, 0      , 0      , &l[4]  , &l[7]  , },
  { "FORCE@off",     OR,    GATE,  0x81, 0      , 0      , &l[5]  , &l[8]  , },
  { "D",           IFUNCT,  GATE,  0x03, &l[15] , &l[10] , &l[9]  , 0      , }, /* 5 pre-installed D function block */
  { "D@",            FF,    GATE,  0x80, &l[9]  , &l[12] , &l[10] , 0      , },
  { "D@expr",        OR,    GATE,  0x81, 0      , 0      , &l[14] , &l[15] , },
  { "D@dcl",         CLK,   CLCKL, 0x81, 0      , 0      , &l[13] , &l[16] , },
  { "SR_",         IFUNCT,  GATE,  0x0f, &l[26] , &l[18] , &l[17] , 0      , }, /* 9 pre-installed SR_ function block */
  { "SR_@",          FF,    GATE,  0x80, &l[17] , &l[20] , &l[18] , 0      , },
  { "SR_@set",       OR,    GATE,  0x81, 0      , 0      , &l[23] , &l[26] , },
  { "SR_@scl",       CLK,   CLCKL, 0x81, 0      , 0      , &l[22] , &l[27] , },
  { "SR_@res",       OR,    GATE,  0x81, 0      , 0      , &l[25] , &l[28] , },
  { "SR_@rcl",       CLK,   CLCKL, 0x81, 0      , 0      , &l[24] , &l[29] , },
  { "DR_",         IFUNCT,  GATE,  0x0f, &l[39] , &l[31] , &l[30] , 0      , }, /* 15 pre-installed DR_ function block */
  { "DR_@",          FF,    GATE,  0x80, &l[30] , &l[33] , &l[31] , 0      , },
  { "DR_@expr",      OR,    GATE,  0x81, 0      , 0      , &l[36] , &l[39] , },
  { "DR_@dcl",       CLK,   CLCKL, 0x81, 0      , 0      , &l[35] , &l[40] , },
  { "DR_@res",       OR,    GATE,  0x81, 0      , 0      , &l[38] , &l[41] , },
  { "DR_@rcl",       CLK,   CLCKL, 0x81, 0      , 0      , &l[37] , &l[42] , },
  { "SRR_",        IFUNCT,  GATE,  0x3f, &l[55] , &l[44] , &l[43] , 0      , }, /* 21 pre-installed SRR_ function block */
  { "SRR_@",         FF,    GATE,  0x80, &l[43] , &l[46] , &l[44] , 0      , },
  { "SRR_@set",      OR,    GATE,  0x81, 0      , 0      , &l[50] , &l[55] , },
  { "SRR_@scl",      CLK,   CLCKL, 0x81, 0      , 0      , &l[49] , &l[56] , },
  { "SRR_@res",      OR,    GATE,  0x81, 0      , 0      , &l[52] , &l[57] , },
  { "SRR_@rcl",      CLK,   CLCKL, 0x81, 0      , 0      , &l[51] , &l[58] , },
  { "SRR_@res2",     OR,    GATE,  0x81, 0      , 0      , &l[54] , &l[59] , },
  { "SRR_@r2cl",     CLK,   CLCKL, 0x81, 0      , 0      , &l[53] , &l[60] , },
  { "DSR_",        IFUNCT,  GATE,  0x3f, &l[73] , &l[62] , &l[61] , 0      , }, /* 29 pre-installed DSR_ function block */
  { "DSR_@",         FF,    GATE,  0x80, &l[61] , &l[64] , &l[62] , 0      , },
  { "DSR_@expr",     OR,    GATE,  0x81, 0      , 0      , &l[68] , &l[73] , },
  { "DSR_@dcl",      CLK,   CLCKL, 0x81, 0      , 0      , &l[67] , &l[74] , },
  { "DSR_@set",      OR,    GATE,  0x81, 0      , 0      , &l[70] , &l[75] , },
  { "DSR_@scl",      CLK,   CLCKL, 0x81, 0      , 0      , &l[69] , &l[76] , },
  { "DSR_@res",      OR,    GATE,  0x81, 0      , 0      , &l[72] , &l[77] , },
  { "DSR_@rcl",      CLK,   CLCKL, 0x81, 0      , 0      , &l[71] , &l[78] , },
  { "SH",          IFUNCT,  ARITH, 0x03, &l[85] , &l[80] , &l[79] , 0      , }, /* 37 pre-installed SH function block */
  { "SH@",           SH,    ARITH, 0x80, &l[79] , &l[82] , &l[80] , 0      , },
  { "SH@dat",        ARN,   ARITH, 0x81, 0      , 0      , &l[84] , &l[85] , },
  { "SH@dcl",        CLK,   CLCKL, 0x81, 0      , 0      , &l[83] , &l[86] , },
  { "SHR_",        IFUNCT,  ARITH, 0x0f, &l[96] , &l[88] , &l[87] , 0      , }, /* 41 pre-installed SHR_ function block */
  { "SHR_@",         SH,    ARITH, 0x80, &l[87] , &l[90] , &l[88] , 0      , },
  { "SHR_@dat",      ARN,   ARITH, 0x81, 0      , 0      , &l[93] , &l[96] , },
  { "SHR_@dcl",      CLK,   CLCKL, 0x81, 0      , 0      , &l[92] , &l[97] , },
  { "SHR_@res",      OR,    GATE,  0x81, 0      , 0      , &l[95] , &l[98] , },
  { "SHR_@rcl",      CLK,   CLCKL, 0x81, 0      , 0      , &l[94] , &l[99] , },
  { "SHSR_",       IFUNCT,  ARITH, 0x3f, &l[112], &l[101], &l[100], 0      , }, /* 47 pre-installed SHSR_ function block */
  { "SHSR_@",        SH,    ARITH, 0x80, &l[100], &l[103], &l[101], 0      , },
  { "SHSR_@dat",     ARN,   ARITH, 0x81, 0      , 0      , &l[107], &l[112], },
  { "SHSR_@dcl",     CLK,   CLCKL, 0x81, 0      , 0      , &l[106], &l[113], },
  { "SHSR_@set",     OR,    GATE,  0x81, 0      , 0      , &l[109], &l[114], },
  { "SHSR_@scl",     CLK,   CLCKL, 0x81, 0      , 0      , &l[108], &l[115], },
  { "SHSR_@res",     OR,    GATE,  0x81, 0      , 0      , &l[111], &l[116], },
  { "SHSR_@rcl",     CLK,   CLCKL, 0x81, 0      , 0      , &l[110], &l[117], },
  { "RISE",        IFUNCT,  GATE,  0x03, &l[124], &l[119], &l[118], 0      , }, /* 55 pre-installed RISE function block */
  { "RISE@",         EF,    GATE,  0x80, &l[118], &l[121], &l[119], 0      , },
  { "RISE@trig",     OR,    GATE,  0x81, 0      , 0      , &l[123], &l[124], },
  { "RISE@clk",      CLK,   CLCKL, 0x81, 0      , 0      , &l[122], &l[125], },
  { "CHANGE",      IFUNCT,  GATE,  0x01, &l[132], &l[127], &l[126], 0      , }, /* 59 pre-installed CHANGE function block */
  { "CHANGE@",       VF,    GATE,  0x80, &l[126], &l[129], &l[127], 0      , },
  { "CHANGE@trig",   OR,    GATE,  0x81, 0      , 0      , &l[131], &l[132], },
  { "CHANGE@clk",    CLK,   CLCKL, 0x81, 0      , 0      , &l[130], &l[133], },
  { "CHANGE2",     IFUNCT,  GATE,  0x02, &l[140], &l[135], &l[134], (List_e*)&b[59], }, /* 63 pre-installed CHANGE2 alternate CHANGE */
  { "CHANGE2@",      VF,    GATE,  0x80, &l[134], &l[137], &l[135], 0      , },
  { "CHANGE2@val",   ARN,   ARITH, 0x81, 0      , 0      , &l[139], &l[140], },
  { "CHANGE2@clk",   CLK,   CLCKL, 0x81, 0      , 0      , &l[138], &l[141], },
  { "CLOCK",       IFUNCT,  CLCKL, 0x03, &l[148], &l[143], &l[142], 0      , }, /* 67 pre-installed CLOCK function block */
  { "CLOCK@",        CLK,   CLCKL, 0x80, &l[142], &l[145], &l[143], 0      , },
  { "CLOCK@trig",    OR,    GATE,  0x81, 0      , 0      , &l[147], &l[148], },
  { "CLOCK@clk",     CLK,   CLCKL, 0x81, 0      , 0      , &l[146], &l[149], },
  { "CLOCK2",      IFUNCT,  CLCKL, 0x0f, &l[159], &l[151], &l[150], (List_e*)&b[67], }, /* 71 pre-installed CLOCK2 alternate CLOCK */
  { "CLOCK2@",       CLK,   CLCKL, 0x80, &l[150], &l[153], &l[151], 0      , },
  { "CLOCK2@trig",   OR,    GATE,  0x81, 0      , 0      , &l[156], &l[159], },
  { "CLOCK2@clk",    CLK,   CLCKL, 0x81, 0      , 0      , &l[155], &l[160], },
  { "CLOCK2@trig2",  OR,    GATE,  0x81, 0      , 0      , &l[158], &l[161], },
  { "CLOCK2@clk2",   CLK,   CLCKL, 0x81, 0      , 0      , &l[157], &l[162], },
  { "TIMER",       IFUNCT,  TIMRL, 0x03, &l[169], &l[164], &l[163], 0      , }, /* 77 pre-installed TIMER function block */
  { "TIMER@",        TIM,   TIMRL, 0x80, &l[163], &l[166], &l[164], 0      , },
  { "TIMER@trig",    OR,    GATE,  0x81, 0      , 0      , &l[168], &l[169], },
  { "TIMER@clk",     CLK,   CLCKL, 0x81, 0      , 0      , &l[167], &l[170], },
  { "TIMER2",      IFUNCT,  TIMRL, 0x0f, &l[180], &l[172], &l[171], (List_e*)&b[77], }, /* 81 pre-installed TIMER2 alternate TIMER */
  { "TIMER2@",       TIM,   TIMRL, 0x80, &l[171], &l[174], &l[172], 0      , },
  { "TIMER2@trig",   OR,    GATE,  0x81, 0      , 0      , &l[177], &l[180], },
  { "TIMER2@clk",    CLK,   CLCKL, 0x81, 0      , 0      , &l[176], &l[181], },
  { "TIMER2@trig2",  OR,    GATE,  0x81, 0      , 0      , &l[179], &l[182], },
  { "TIMER2@clk2",   CLK,   CLCKL, 0x81, 0      , 0      , &l[178], &l[183], },
  { "TIMER1",      IFUNCT,  TIMRL, 0x03, &l[190], &l[185], &l[184], 0      , }, /* 87 pre-installed TIMER1 function block */
  { "TIMER1@",       TIM,   TIMRL, 0x80, &l[184], &l[187], &l[185], 0      , },
  { "TIMER1@trig",   OR,    GATE,  0x81, 0      , 0      , &l[189], &l[190], },
  { "TIMER1@clk",    CLK,   CLCKL, 0x81, 0      , 0      , &l[188], &l[191], },
  { "TIMER12",     IFUNCT,  TIMRL, 0x0f, &l[201], &l[193], &l[192], (List_e*)&b[87], }, /* 91 pre-installed TIMER12 alternate TIMER1 */
  { "TIMER12@",      TIM,   TIMRL, 0x80, &l[192], &l[195], &l[193], 0      , },
  { "TIMER12@trig",  OR,    GATE,  0x81, 0      , 0      , &l[198], &l[201], },
  { "TIMER12@clk",   CLK,   CLCKL, 0x81, 0      , 0      , &l[197], &l[202], },
  { "TIMER12@trig2", OR,    GATE,  0x81, 0      , 0      , &l[200], &l[203], },
  { "TIMER12@clk2",  CLK,   CLCKL, 0x81, 0      , 0      , &l[199], &l[204], },
									   /* 97 pre-installed BuiltIn function Symbols */
#else	/* not BOOT_COMPILE */
    /* name	type	ftype	uVal	*/
  { "FORCE",	KEYW,	0,	BFORCE,	0,	0,	0,	0,	}, /* FORCE function - generates LATCH */
  { "D",	KEYW,	D_FF,	BLTIN1,	0,	0,	0,	0,	}, /* D flip-flop */
  { "SR_",	KEYW,	S_FF,	BLTIN2,	0,	0,	0,	0,	}, /* R_FF for reset master */
  { "DR_",	KEYW,	D_FF,	BLTIN2,	0,	0,	0,	0,	}, /* D flip-flop with simple reset */
  { "SRR_",	KEYW,	S_FF,	BLTIN3,	0,	0,	0,	0,	}, /* R_FF for reset master */
  { "DSR_",	KEYW,	D_FF,	BLTIN3,	0,	0,	0,	0,	}, /* D flip-flop with simple set/reset */
  { "SH",	KEYW,	D_SH,	BLTIN1,	0,	0,	0,	0,	}, /* sample and hold */
  { "SHR_",	KEYW,	D_SH,	BLTIN2,	0,	0,	0,	0,	}, /* sample and hold with simple reset */
  { "SHSR_",	KEYW,	D_SH,	BLTIN3,	0,	0,	0,	0,	}, /* sample and hold with simple set/reset */
  { "RISE",	KEYW,	RI_BIT,	BLTIN1,	0,	0,	0,	0,	}, /* pulse on digital rising edge */
  { "CHANGE",	KEYW,	CH_BIT,	BLTINC,	0,	0,	0,	0,	}, /* pulse on digital (CH_BIT) or analog (CH_AR) change */
  { "CLOCK",	KEYW,	CLCK,	CBLTIN,	0,	0,	0,	0,	}, /* clock */
  { "TIMER",	KEYW,	TIMR,	TBLTIN,	0,	0,	0,	0,	}, /* normal timer with preset off 0 */
  { "TIMER1",	KEYW,	TIMR,	TBLTI1,	0,	0,	0,	0,	}, /* alternate timer with preset off 1 */
#endif	/* not BOOT_COMPILE */
  { "if",	KEYW,	F_CF,	IF,	0,	0,	0,	0,	},
  { "else",	KEYW,	F_CE,	ELSE,	0,	0,	0,	0,	},
  { "switch",	KEYW,	F_SW,	SWITCH,	0,	0,	0,	0,	},
  { "extern",	KEYW,	0,	EXTERN,	0,	0,	0,	0,	},
  { "assign",	KEYW,	0,	ASSIGN,	0,	0,	0,	0,	},
  { "return",	KEYW,	0,	RETURN,	0,	0,	0,	0,	},
  { "no",	KEYW,	0,	USE,	0,	0,	0,	0,	}, /* turn off use */
  { "use",	KEYW,	1,	USE,	0,	0,	0,	0,	}, /* turn on use */
  { "alias",	KEYW,	0,     USETYPE,	0,	0,	0,	0,	}, /* check that USETYPE < MAXUSETYPE */
  { "strict",	KEYW,	1,     USETYPE,	0,	0,	0,	0,	}, /* MAXUSETYPE 2 */
  { "imm",	KEYW,	0,	IMM,	0,	0,	0,	0,	},
  { "immC",	KEYW,	1,	IMM,	0,	0,	0,	0,	},
  { "void",	KEYW,	UDFA,	VOID,	0,	0,	0,	0,	},
  { "bit",	KEYW,	GATE,	TYPE,	0,	0,	0,	0,	},
  { "int",	KEYW,	ARITH,	TYPE,	0,	0,	0,	0,	},
  { "clock",	KEYW,	CLCKL,	TYPE,	0,	0,	0,	0,	},
  { "timer",	KEYW,	TIMRL,	TYPE,	0,	0,	0,	0,	},
  { "sizeof",	KEYW,	0,	SIZEOF,	0,	0,	0,	0,	},
  { "this",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	}, /* only used in function block definitions */
  { "auto",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	}, /* C keywords to cause syntax errors if used in iC */
  { "break",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "case",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "char",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "const",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "continue",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "default",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "do",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "double",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "enum",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "float",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "for",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "goto",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "long",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "register",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "short",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "signed",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "static",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "struct",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "typedef",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "union",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "unsigned",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "volatile",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "while",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "fortran",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "asm",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	},
  { "FOR",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	}, /* keyword used in immac FOR loops */
  { "IF",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	}, /* keyword used in immac IF statements */
  { "ELSE",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	}, /* keyword used in immac ELSE statements */
  { "ELSIF",	KEYW,	0,	LEXERR,	0,	0,	0,	0,	}, /* keyword used in immac ELSIF statements */
  { "iC_Gt",	CTYPE,	0,   YYERRCODE,	0,	0,	0,	0,	}, /* initial Gate C-type from icg.h */
  { "iConst",	NCONST, ARITH,	0,	0,	0,	0,	0,	}, /* Symbol "iConst" must be second last non-zero entry */
  { "iClock",	CLK,	CLCKL,	0,	0,	0,	0,	0,	}, /* Symbol "iClock" must be last non-zero entry */
  { 0,		0,	0,	0,	0,	0,	0,	0,	}, /* terminate initialisation with name == 0 */
}; /* b[] */

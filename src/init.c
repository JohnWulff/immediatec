static const char init_c[] =
"@(#)$Id: init.c 1.49 $";
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
#include	"comp.h"
#include	"comp.tab.h"

Symbol *	iclock;			/* default clock */
Symbol *	icerr = 0;		/* pointer to Symbol "iCerr" */
					/* ### insert generated data here ### */
static BuiltIn	b[];
int		iC_genCount = 0;	/* count pre-compiled ar expressions - if any */

#ifndef BOOT_COMPILE
static List_e	l[];

/********************************************************************
 * Pre-compiled built-in functions produced in functionDefinition()
 *
 * Bootstrapping the original built-in functions, requires an older
 * version of the compiler which compiles built-ins directly.
 *
 * if BOOT_COMPILE is defined this direct compilation is left active
 * and any pre-compiled functions are left out of init.c.
 *******************************************************************/

/********************************************************************
 *
 *	Internal symbols for the pre-compiled built-in function blocks
 *
 *******************************************************************/


static Symbol	Da_1       = { .type=OR , .ftype=D_FF  , .fm=0x41, .u_blist=&l[13] };

static Symbol	DS_a_2     = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[24] };
static Symbol	DS_a_1     = { .type=OR , .ftype=D_FF  , .fm=0x41, .u_blist=&l[22], .next=&DS_a_2 };

static Symbol	DR_a_2     = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[37] };
static Symbol	DR_a_1     = { .type=OR , .ftype=D_FF  , .fm=0x41, .u_blist=&l[35], .next=&DR_a_2 };

static Symbol	DSR_a_3    = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[53] };
static Symbol	DSR_a_2    = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[51], .next=&DSR_a_3 };
static Symbol	DSR_a_1    = { .type=OR , .ftype=D_FF  , .fm=0x41, .u_blist=&l[49], .next=&DSR_a_2 };

static Symbol	SR_a_2     = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[68] };
static Symbol	SR_a_1     = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[66], .next=&SR_a_2 };

static Symbol	SRR_a_3    = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[84] };
static Symbol	SRR_a_2    = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[82], .next=&SRR_a_3 };
static Symbol	SRR_a_1    = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[80], .next=&SRR_a_2 };

static Symbol	SHa_1      = { .type=ARN, .ftype=D_SH  , .fm=0x41, .u_blist=&l[96] };

static Symbol	SHR_a_2    = { .type=OR , .ftype=R_SH  , .fm=0x41, .u_blist=&l[107] };
static Symbol	SHR_a_1    = { .type=ARN, .ftype=D_SH  , .fm=0x41, .u_blist=&l[105], .next=&SHR_a_2 };

static Symbol	SHSR_a_3   = { .type=OR , .ftype=R_SH  , .fm=0x41, .u_blist=&l[123] };
static Symbol	SHSR_a_2   = { .type=OR , .ftype=S_SH  , .fm=0x41, .u_blist=&l[121], .next=&SHSR_a_3 };
static Symbol	SHSR_a_1   = { .type=ARN, .ftype=D_SH  , .fm=0x41, .u_blist=&l[119], .next=&SHSR_a_2 };

static Symbol	RISEa_1    = { .type=OR , .ftype=RI_BIT, .fm=0x41, .u_blist=&l[135] };

static Symbol	CHANGEa_1  = { .type=OR , .ftype=CH_BIT, .fm=0x41, .u_blist=&l[143] };

static Symbol	CHANGE2a_1 = { .type=ARN, .ftype=CH_AR , .fm=0x41, .u_blist=&l[151] };

static Symbol	CLOCKa_1   = { .type=OR , .ftype=CLCK  , .fm=0x41, .u_blist=&l[159] };

static Symbol	CLOCK2a_2  = { .type=OR , .ftype=CLCK  , .fm=0x41, .u_blist=&l[170] };
static Symbol	CLOCK2a_1  = { .type=OR , .ftype=CLCK  , .fm=0x41, .u_blist=&l[168], .next=&CLOCK2a_2 };

static Symbol	TIMERa_1   = { .type=OR , .ftype=TIMR  , .fm=0x41, .u_blist=&l[180] };

static Symbol	TIMER02a_2 = { .type=OR , .ftype=TIMR  , .fm=0x41, .u_blist=&l[191] };
static Symbol	TIMER02a_1 = { .type=OR , .ftype=TIMR  , .fm=0x41, .u_blist=&l[189], .next=&TIMER02a_2 };

static Symbol	TIMER1a_1  = { .type=OR , .ftype=TIMR  , .fm=0x41, .u_blist=&l[201] };

static Symbol	TIMER12a_2 = { .type=OR , .ftype=TIMR  , .fm=0x41, .u_blist=&l[212] };
static Symbol	TIMER12a_1 = { .type=OR , .ftype=TIMR  , .fm=0x41, .u_blist=&l[210], .next=&TIMER12a_2 };

static Symbol	DSa_1      = { .type=OR , .ftype=D_FF  , .fm=0x41, .u_blist=&l[224] };
static Symbol	DSa_3      = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[235] };
static Symbol	DSa_2      = { .type=OR , .ftype=D_FF  , .fm=0x41, .u_blist=&l[232], .next=&DSa_3 };
static Symbol	DSa_5      = { .type=AND, .ftype=GATE  , .fm=0x41, .u_blist=&l[230], .next=&DSa_2 };
static Symbol	DSa_4      = { .type=AND, .ftype=GATE  , .fm=0x41, .u_blist=&l[228], .next=&DSa_5 };

static Symbol	DRa_2      = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[253] };
static Symbol	DRa_1      = { .type=OR , .ftype=D_FF  , .fm=0x41, .u_blist=&l[250], .next=&DRa_2 };
static Symbol	DRa_4      = { .type=AND, .ftype=GATE  , .fm=0x41, .u_blist=&l[248], .next=&DRa_1 };
static Symbol	DRa_3      = { .type=AND, .ftype=GATE  , .fm=0x41, .u_blist=&l[246], .next=&DRa_4 };

static Symbol	DSRa_1     = { .type=FF , .ftype=GATE  , .fm=0x41, .u_blist=&l[268] };
static Symbol	DSRa_2     = { .type=OR , .ftype=D_FF  , .fm=0x41, .u_blist=&l[266], .next=&DSRa_1 };
static Symbol	DSRa_5     = { .type=AND, .ftype=R_FF  , .fm=0x41, .u_blist=&l[282] };
static Symbol	DSRa_4     = { .type=AND, .ftype=S_FF  , .fm=0x41, .u_blist=&l[279], .next=&DSRa_5 };
static Symbol	DSRa_3     = { .type=OR , .ftype=D_FF  , .fm=0x41, .u_blist=&l[276], .next=&DSRa_4 };
static Symbol	DSRa_7     = { .type=AND, .ftype=GATE  , .fm=0x41, .u_blist=&l[274], .next=&DSRa_3 };
static Symbol	DSRa_6     = { .type=AND, .ftype=GATE  , .fm=0x41, .u_blist=&l[272], .next=&DSRa_7 };

static Symbol	SRa_2      = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[298] };
static Symbol	SRa_1      = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[296], .next=&SRa_2 };

static Symbol	SRRa_3     = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[314] };
static Symbol	SRRa_2     = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[312], .next=&SRRa_3 };
static Symbol	SRRa_1     = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[310], .next=&SRRa_2 };

static Symbol	SRXa_2     = { .type=AND, .ftype=R_FF  , .fm=0x41, .u_blist=&l[330] };
static Symbol	SRXa_1     = { .type=AND, .ftype=S_FF  , .fm=0x41, .u_blist=&l[327], .next=&SRXa_2 };

static Symbol	JKa_2      = { .type=AND, .ftype=R_FF  , .fm=0x41, .u_blist=&l[345] };
static Symbol	JKa_1      = { .type=AND, .ftype=S_FF  , .fm=0x41, .u_blist=&l[342], .next=&JKa_2 };

static Symbol	SHRa_2     = { .type=OR , .ftype=R_SH  , .fm=0x41, .u_blist=&l[362] };
static Symbol	SHRa_1     = { .type=ARN, .ftype=D_SH  , .fm=0x41, .u_blist=&l[358], .next=&SHRa_2 };
static Symbol	SHRa_3     = { .type=OR , .ftype=ARITH , .fm=0x41, .u_blist=&l[357], .next=&SHRa_1 };

static Symbol	SHSRa_3    = { .type=AND, .ftype=R_SH  , .fm=0x41, .u_blist=&l[383] };
static Symbol	SHSRa_2    = { .type=AND, .ftype=S_SH  , .fm=0x41, .u_blist=&l[380], .next=&SHSRa_3 };
static Symbol	SHSRa_1    = { .type=ARN, .ftype=D_SH  , .fm=0x41, .u_blist=&l[376], .next=&SHSRa_2 };
static Symbol	SHSRa_4    = { .type=OR , .ftype=ARITH , .fm=0x41, .u_blist=&l[374], .next=&SHSRa_1 };

static Symbol	STa_2      = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[399] };
static Symbol	STa_1      = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[397], .next=&STa_2 };

static Symbol	SRTa_3     = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[414] };
static Symbol	SRTa_2     = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[412], .next=&SRTa_3 };
static Symbol	SRTa_1     = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[410], .next=&SRTa_2 };

static Symbol	FALLa_1    = { .type=OR , .ftype=RI_BIT, .fm=0x41, .u_blist=&l[425] };


static Symbol	DLATCHa_1  = { .type=LATCH, .ftype=D_FF  , .fm=0x41, .u_blist=&l[441] };


/********************************************************************
 *
 *	Array ArExpr[][] for setting up initial arithmetic expressions
 *	for numeric pre-compiled function blocks
 *
 *******************************************************************/

static char *	ArExpr[][2] = {
    {
	"\377\001?\377\002:\377\003",	/* (1) */
	"#line 139 \"init_t.ic\"	/* in pre-compiled function block SHR */",
    },
    { NULL, NULL, },			/* terminator */
};

#define GEN_MAX	2
char *	iC_genName;			/* record file name of pre-compiled sources */
int	iC_genLineNums[GEN_MAX];	/* record line numbers in pre-compiled sources */

#define FO	FUN_OFFSET

/********************************************************************
 *
 *	Actual links for the pre-compiled built-in function blocks
 *	Pointers le_sym to symbols in the Symbol Table are initialised
 *	from link1 and link2 in b[] and the extra links in le_first
 *
 *******************************************************************/

static List_e	l[] = {
/* index    le_sym       le_val     le_next  le_first        le_last	** ==>  name */

/*   0 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  FORCE */
/*   1 */ { 0          , 0        , &l[2]  , 0             , 0 },	/* ==>  FORCE@ */
/*   2 */ { 0          , 3        , 0      , 0             , 0 },	/* ==>  no var list */
/*   3 */ { 0          , 0        , &l[4]  , 0             , 0 },	/* ==>  FORCE@arg1 */
/*   4 */ { 0          , 0        , &l[5]  , 0             , 0 },	/* ==>  FORCE@on */
/*   5 */ { 0          , 1        , 0      , 0             , 0 },	/* ==> ~FORCE@off */
/*   6 */ { 0          , 0        , &l[7]  , 0             , 0 },	/* ==>  FORCE@arg1 */
/*   7 */ { 0          , 0        , &l[8]  , 0             , 0 },	/* ==>  FORCE@on */
/*   8 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  FORCE@off */

/*   9 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  D */
/*  10 */ { 0          , 0        , &l[11] , 0             , 0 },	/* ==>  D@ */
/*  11 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/*  12 */ { &Da_1      , 0        , 0      , (char*)&l[11] , 0 },	/* ==>  Da_1 */
/*  13 */ { 0          , 0        , &l[14] , 0             , 0 },	/* ==>  D@dcl */
/*  14 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  D@expr */
/*  15 */ { 0          , 0        , &l[16] , 0             , 0 },	/* ==>  D@expr */
/*  16 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  D@dcl */

/*  17 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DS_ */
/*  18 */ { 0          , 0        , &l[19] , 0             , 0 },	/* ==>  DS_@ */
/*  19 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/*  20 */ { &DS_a_1    , 0        , &l[21] , (char*)&l[19] , 0 },	/* ==>  DS_a_1 */
/*  21 */ { &DS_a_2    , 0        , 0      , 0             , 0 },	/* ==>  DS_a_2 */
/*  22 */ { 0          , 0        , &l[23] , 0             , 0 },	/* ==>  DS_@dcl */
/*  23 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DS_@expr */
/*  24 */ { 0          , 0        , &l[25] , 0             , 0 },	/* ==>  DS_@scl */
/*  25 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DS_@set */
/*  26 */ { 0          , 0        , &l[27] , 0             , 0 },	/* ==>  DS_@expr */
/*  27 */ { 0          , 0        , &l[28] , 0             , 0 },	/* ==>  DS_@dcl */
/*  28 */ { 0          , 0        , &l[29] , 0             , 0 },	/* ==>  DS_@set */
/*  29 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DS_@scl */

/*  30 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DR_ */
/*  31 */ { 0          , 0        , &l[32] , 0             , 0 },	/* ==>  DR_@ */
/*  32 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/*  33 */ { &DR_a_1    , 0        , &l[34] , (char*)&l[32] , 0 },	/* ==>  DR_a_1 */
/*  34 */ { &DR_a_2    , 0        , 0      , 0             , 0 },	/* ==>  DR_a_2 */
/*  35 */ { 0          , 0        , &l[36] , 0             , 0 },	/* ==>  DR_@dcl */
/*  36 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DR_@expr */
/*  37 */ { 0          , 0        , &l[38] , 0             , 0 },	/* ==>  DR_@rcl */
/*  38 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DR_@res */
/*  39 */ { 0          , 0        , &l[40] , 0             , 0 },	/* ==>  DR_@expr */
/*  40 */ { 0          , 0        , &l[41] , 0             , 0 },	/* ==>  DR_@dcl */
/*  41 */ { 0          , 0        , &l[42] , 0             , 0 },	/* ==>  DR_@res */
/*  42 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DR_@rcl */

/*  43 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR_ */
/*  44 */ { 0          , 0        , &l[45] , 0             , 0 },	/* ==>  DSR_@ */
/*  45 */ { 0          , 6        , 0      , 0             , 0 },	/* ==>  var list */
/*  46 */ { &DSR_a_1   , 0        , &l[47] , (char*)&l[45] , 0 },	/* ==>  DSR_a_1 */
/*  47 */ { &DSR_a_2   , 0        , &l[48] , 0             , 0 },	/* ==>  DSR_a_2 */
/*  48 */ { &DSR_a_3   , 0        , 0      , 0             , 0 },	/* ==>  DSR_a_3 */
/*  49 */ { 0          , 0        , &l[50] , 0             , 0 },	/* ==>  DSR_@dcl */
/*  50 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR_@expr */
/*  51 */ { 0          , 0        , &l[52] , 0             , 0 },	/* ==>  DSR_@scl */
/*  52 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR_@set */
/*  53 */ { 0          , 0        , &l[54] , 0             , 0 },	/* ==>  DSR_@rcl */
/*  54 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR_@res */
/*  55 */ { 0          , 0        , &l[56] , 0             , 0 },	/* ==>  DSR_@expr */
/*  56 */ { 0          , 0        , &l[57] , 0             , 0 },	/* ==>  DSR_@dcl */
/*  57 */ { 0          , 0        , &l[58] , 0             , 0 },	/* ==>  DSR_@set */
/*  58 */ { 0          , 0        , &l[59] , 0             , 0 },	/* ==>  DSR_@scl */
/*  59 */ { 0          , 0        , &l[60] , 0             , 0 },	/* ==>  DSR_@res */
/*  60 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR_@rcl */

/*  61 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR_ */
/*  62 */ { 0          , 0        , &l[63] , 0             , 0 },	/* ==>  SR_@ */
/*  63 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/*  64 */ { &SR_a_1    , 0        , &l[65] , (char*)&l[63] , 0 },	/* ==>  SR_a_1 */
/*  65 */ { &SR_a_2    , 0        , 0      , 0             , 0 },	/* ==>  SR_a_2 */
/*  66 */ { 0          , 0        , &l[67] , 0             , 0 },	/* ==>  SR_@scl */
/*  67 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR_@set */
/*  68 */ { 0          , 0        , &l[69] , 0             , 0 },	/* ==>  SR_@rcl */
/*  69 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR_@res */
/*  70 */ { 0          , 0        , &l[71] , 0             , 0 },	/* ==>  SR_@set */
/*  71 */ { 0          , 0        , &l[72] , 0             , 0 },	/* ==>  SR_@scl */
/*  72 */ { 0          , 0        , &l[73] , 0             , 0 },	/* ==>  SR_@res */
/*  73 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR_@rcl */

/*  74 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR_ */
/*  75 */ { 0          , 0        , &l[76] , 0             , 0 },	/* ==>  SRR_@ */
/*  76 */ { 0          , 6        , 0      , 0             , 0 },	/* ==>  var list */
/*  77 */ { &SRR_a_1   , 0        , &l[78] , (char*)&l[76] , 0 },	/* ==>  SRR_a_1 */
/*  78 */ { &SRR_a_2   , 0        , &l[79] , 0             , 0 },	/* ==>  SRR_a_2 */
/*  79 */ { &SRR_a_3   , 0        , 0      , 0             , 0 },	/* ==>  SRR_a_3 */
/*  80 */ { 0          , 0        , &l[81] , 0             , 0 },	/* ==>  SRR_@scl */
/*  81 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR_@set */
/*  82 */ { 0          , 0        , &l[83] , 0             , 0 },	/* ==>  SRR_@rcl */
/*  83 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR_@res */
/*  84 */ { 0          , 0        , &l[85] , 0             , 0 },	/* ==>  SRR_@r2cl */
/*  85 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR_@res2 */
/*  86 */ { 0          , 0        , &l[87] , 0             , 0 },	/* ==>  SRR_@set */
/*  87 */ { 0          , 0        , &l[88] , 0             , 0 },	/* ==>  SRR_@scl */
/*  88 */ { 0          , 0        , &l[89] , 0             , 0 },	/* ==>  SRR_@res */
/*  89 */ { 0          , 0        , &l[90] , 0             , 0 },	/* ==>  SRR_@rcl */
/*  90 */ { 0          , 0        , &l[91] , 0             , 0 },	/* ==>  SRR_@res2 */
/*  91 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR_@r2cl */

/*  92 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SH */
/*  93 */ { 0          , 0        , &l[94] , 0             , 0 },	/* ==>  SH@ */
/*  94 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/*  95 */ { &SHa_1     , 0        , 0      , (char*)&l[94] , 0 },	/* ==>  SHa_1 */
/*  96 */ { 0          , 0        , &l[97] , 0             , 0 },	/* ==>  SH@dcl */
/*  97 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SH@dat */
/*  98 */ { 0          , 0        , &l[99] , 0             , 0 },	/* ==>  SH@dat */
/*  99 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SH@dcl */

/* 100 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR_ */
/* 101 */ { 0          , 0        , &l[102], 0             , 0 },	/* ==>  SHR_@ */
/* 102 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 103 */ { &SHR_a_1   , 0        , &l[104], (char*)&l[102], 0 },	/* ==>  SHR_a_1 */
/* 104 */ { &SHR_a_2   , 0        , 0      , 0             , 0 },	/* ==>  SHR_a_2 */
/* 105 */ { 0          , 0        , &l[106], 0             , 0 },	/* ==>  SHR_@dcl */
/* 106 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR_@dat */
/* 107 */ { 0          , 0        , &l[108], 0             , 0 },	/* ==>  SHR_@rcl */
/* 108 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR_@res */
/* 109 */ { 0          , 0        , &l[110], 0             , 0 },	/* ==>  SHR_@dat */
/* 110 */ { 0          , 0        , &l[111], 0             , 0 },	/* ==>  SHR_@dcl */
/* 111 */ { 0          , 0        , &l[112], 0             , 0 },	/* ==>  SHR_@res */
/* 112 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR_@rcl */

/* 113 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR_ */
/* 114 */ { 0          , 0        , &l[115], 0             , 0 },	/* ==>  SHSR_@ */
/* 115 */ { 0          , 6        , 0      , 0             , 0 },	/* ==>  var list */
/* 116 */ { &SHSR_a_1  , 0        , &l[117], (char*)&l[115], 0 },	/* ==>  SHSR_a_1 */
/* 117 */ { &SHSR_a_2  , 0        , &l[118], 0             , 0 },	/* ==>  SHSR_a_2 */
/* 118 */ { &SHSR_a_3  , 0        , 0      , 0             , 0 },	/* ==>  SHSR_a_3 */
/* 119 */ { 0          , 0        , &l[120], 0             , 0 },	/* ==>  SHSR_@dcl */
/* 120 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR_@dat */
/* 121 */ { 0          , 0        , &l[122], 0             , 0 },	/* ==>  SHSR_@scl */
/* 122 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR_@set */
/* 123 */ { 0          , 0        , &l[124], 0             , 0 },	/* ==>  SHSR_@rcl */
/* 124 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR_@res */
/* 125 */ { 0          , 0        , &l[126], 0             , 0 },	/* ==>  SHSR_@dat */
/* 126 */ { 0          , 0        , &l[127], 0             , 0 },	/* ==>  SHSR_@dcl */
/* 127 */ { 0          , 0        , &l[128], 0             , 0 },	/* ==>  SHSR_@set */
/* 128 */ { 0          , 0        , &l[129], 0             , 0 },	/* ==>  SHSR_@scl */
/* 129 */ { 0          , 0        , &l[130], 0             , 0 },	/* ==>  SHSR_@res */
/* 130 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR_@rcl */

/* 131 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  RISE */
/* 132 */ { 0          , 0        , &l[133], 0             , 0 },	/* ==>  RISE@ */
/* 133 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 134 */ { &RISEa_1   , 0        , 0      , (char*)&l[133], 0 },	/* ==>  RISEa_1 */
/* 135 */ { 0          , 0        , &l[136], 0             , 0 },	/* ==>  RISE@clk */
/* 136 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  RISE@trig */
/* 137 */ { 0          , 0        , &l[138], 0             , 0 },	/* ==>  RISE@trig */
/* 138 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  RISE@clk */

/* 139 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CHANGE */
/* 140 */ { 0          , 0        , &l[141], 0             , 0 },	/* ==>  CHANGE@ */
/* 141 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 142 */ { &CHANGEa_1 , 0        , 0      , (char*)&l[141], 0 },	/* ==>  CHANGEa_1 */
/* 143 */ { 0          , 0        , &l[144], 0             , 0 },	/* ==>  CHANGE@clk */
/* 144 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CHANGE@trig */
/* 145 */ { 0          , 0        , &l[146], 0             , 0 },	/* ==>  CHANGE@trig */
/* 146 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CHANGE@clk */

/* 147 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CHANGE2 */
/* 148 */ { 0          , 0        , &l[149], 0             , 0 },	/* ==>  CHANGE2@ */
/* 149 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 150 */ { &CHANGE2a_1, 0        , 0      , (char*)&l[149], 0 },	/* ==>  CHANGE2a_1 */
/* 151 */ { 0          , 0        , &l[152], 0             , 0 },	/* ==>  CHANGE2@clk */
/* 152 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CHANGE2@val */
/* 153 */ { 0          , 0        , &l[154], 0             , 0 },	/* ==>  CHANGE2@val */
/* 154 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CHANGE2@clk */

/* 155 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK */
/* 156 */ { 0          , 0        , &l[157], 0             , 0 },	/* ==>  CLOCK@ */
/* 157 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 158 */ { &CLOCKa_1  , 0        , 0      , (char*)&l[157], 0 },	/* ==>  CLOCKa_1 */
/* 159 */ { 0          , 0        , &l[160], 0             , 0 },	/* ==>  CLOCK@clk */
/* 160 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK@trig */
/* 161 */ { 0          , 0        , &l[162], 0             , 0 },	/* ==>  CLOCK@trig */
/* 162 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK@clk */

/* 163 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK2 */
/* 164 */ { 0          , 0        , &l[165], 0             , 0 },	/* ==>  CLOCK2@ */
/* 165 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 166 */ { &CLOCK2a_1 , 0        , &l[167], (char*)&l[165], 0 },	/* ==>  CLOCK2a_1 */
/* 167 */ { &CLOCK2a_2 , 0        , 0      , 0             , 0 },	/* ==>  CLOCK2a_2 */
/* 168 */ { 0          , 0        , &l[169], 0             , 0 },	/* ==>  CLOCK2@clk */
/* 169 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK2@trig */
/* 170 */ { 0          , 0        , &l[171], 0             , 0 },	/* ==>  CLOCK2@clk2 */
/* 171 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK2@trig2 */
/* 172 */ { 0          , 0        , &l[173], 0             , 0 },	/* ==>  CLOCK2@trig */
/* 173 */ { 0          , 0        , &l[174], 0             , 0 },	/* ==>  CLOCK2@clk */
/* 174 */ { 0          , 0        , &l[175], 0             , 0 },	/* ==>  CLOCK2@trig2 */
/* 175 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK2@clk2 */

/* 176 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER */
/* 177 */ { 0          , 0        , &l[178], 0             , 0 },	/* ==>  TIMER@ */
/* 178 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 179 */ { &TIMERa_1  , 0        , 0      , (char*)&l[178], 0 },	/* ==>  TIMERa_1 */
/* 180 */ { 0          , 0        , &l[181], 0             , 0 },	/* ==>  TIMER@clk */
/* 181 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER@trig */
/* 182 */ { 0          , 0        , &l[183], 0             , 0 },	/* ==>  TIMER@trig */
/* 183 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER@clk */

/* 184 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER02 */
/* 185 */ { 0          , 0        , &l[186], 0             , 0 },	/* ==>  TIMER02@ */
/* 186 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 187 */ { &TIMER02a_1, 0        , &l[188], (char*)&l[186], 0 },	/* ==>  TIMER02a_1 */
/* 188 */ { &TIMER02a_2, 0        , 0      , 0             , 0 },	/* ==>  TIMER02a_2 */
/* 189 */ { 0          , 0        , &l[190], 0             , 0 },	/* ==>  TIMER02@clk */
/* 190 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER02@trig */
/* 191 */ { 0          , 0        , &l[192], 0             , 0 },	/* ==>  TIMER02@clk2 */
/* 192 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER02@trig2 */
/* 193 */ { 0          , 0        , &l[194], 0             , 0 },	/* ==>  TIMER02@trig */
/* 194 */ { 0          , 0        , &l[195], 0             , 0 },	/* ==>  TIMER02@clk */
/* 195 */ { 0          , 0        , &l[196], 0             , 0 },	/* ==>  TIMER02@trig2 */
/* 196 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER02@clk2 */

/* 197 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER1 */
/* 198 */ { 0          , 0        , &l[199], 0             , 0 },	/* ==>  TIMER1@ */
/* 199 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 200 */ { &TIMER1a_1 , 1        , 0      , (char*)&l[199], 0 },	/* ==> ~TIMER1a_1 */
/* 201 */ { 0          , 0        , &l[202], 0             , 0 },	/* ==>  TIMER1@clk */
/* 202 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER1@trig */
/* 203 */ { 0          , 0        , &l[204], 0             , 0 },	/* ==>  TIMER1@trig */
/* 204 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER1@clk */

/* 205 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER12 */
/* 206 */ { 0          , 0        , &l[207], 0             , 0 },	/* ==>  TIMER12@ */
/* 207 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 208 */ { &TIMER12a_1, 1        , &l[209], (char*)&l[207], 0 },	/* ==> ~TIMER12a_1 */
/* 209 */ { &TIMER12a_2, 0        , 0      , 0             , 0 },	/* ==>  TIMER12a_2 */
/* 210 */ { 0          , 0        , &l[211], 0             , 0 },	/* ==>  TIMER12@clk */
/* 211 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER12@trig */
/* 212 */ { 0          , 0        , &l[213], 0             , 0 },	/* ==>  TIMER12@clk2 */
/* 213 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER12@trig2 */
/* 214 */ { 0          , 0        , &l[215], 0             , 0 },	/* ==>  TIMER12@trig */
/* 215 */ { 0          , 0        , &l[216], 0             , 0 },	/* ==>  TIMER12@clk */
/* 216 */ { 0          , 0        , &l[217], 0             , 0 },	/* ==>  TIMER12@trig2 */
/* 217 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER12@clk2 */

/* 218 */ { 0          , 0        , &l[219], 0             , 0 },	/* ==>  DS@i */
/* 219 */ { 0          , 5        , &l[221], 0             , 0 },	/* ==>  var list */
/* 220 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DS */
/* 221 */ { 0          , 0        , &l[222], 0             , 0 },	/* ==>  DS@ */
/* 222 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  var list */
/* 223 */ { &DSa_1     , 0        , 0      , (char*)&l[219], 0 },	/* ==>  DSa_1 */
/* 224 */ { 0          , 0        , &l[225], 0             , 0 },	/* ==>  DS@scl */
/* 225 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DS@set */
/* 226 */ { &DSa_2     , 0        , &l[227], 0             , 0 },	/* ==>  DSa_2 */
/* 227 */ { &DSa_3     , 0        , 0      , 0             , 0 },	/* ==>  DSa_3 */
/* 228 */ { 0          , 0        , &l[229], 0             , 0 },	/* ==>  DS@dat */
/* 229 */ { 0          , 1        , 0      , (char*)&l[218], 0 },	/* ==> ~DS@i */
/* 230 */ { 0          , 0        , &l[231], 0             , 0 },	/* ==>  DS@ */
/* 231 */ { 0          , 0        , 0      , (char*)&l[229], 0 },	/* ==>  DS@i */
/* 232 */ { 0          , 0        , &l[233], 0             , 0 },	/* ==>  DS@dcl */
/* 233 */ { &DSa_4     , 0        , &l[234], (char*)&l[222], 0 },	/* ==>  DSa_4 */
/* 234 */ { &DSa_5     , 0        , 0      , 0             , 0 },	/* ==>  DSa_5 */
/* 235 */ { 0          , 0        , &l[236], (char*)&l[224], 0 },	/* ==>  DS@scl */
/* 236 */ { 0          , 0        , 0      , (char*)&l[225], 0 },	/* ==>  DS@set */
/* 237 */ { 0          , 0        , &l[238], 0             , 0 },	/* ==>  DS@dat */
/* 238 */ { 0          , 0        , &l[239], 0             , 0 },	/* ==>  DS@dcl */
/* 239 */ { 0          , 0        , &l[240], 0             , 0 },	/* ==>  DS@set */
/* 240 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DS@scl */

/* 241 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DR */
/* 242 */ { 0          , 0        , &l[243], 0             , 0 },	/* ==>  DR@ */
/* 243 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 244 */ { &DRa_1     , 0        , &l[245], 0             , 0 },	/* ==>  DRa_1 */
/* 245 */ { &DRa_2     , 0        , 0      , 0             , 0 },	/* ==>  DRa_2 */
/* 246 */ { 0          , 0        , &l[247], 0             , 0 },	/* ==>  DR@dat */
/* 247 */ { 0          , 1        , 0      , 0             , 0 },	/* ==> ~DR@res */
/* 248 */ { 0          , 0        , &l[249], 0             , 0 },	/* ==>  DR@ */
/* 249 */ { 0          , 0        , 0      , (char*)&l[247], 0 },	/* ==>  DR@res */
/* 250 */ { 0          , 0        , &l[251], 0             , 0 },	/* ==>  DR@dcl */
/* 251 */ { &DRa_3     , 0        , &l[252], (char*)&l[243], 0 },	/* ==>  DRa_3 */
/* 252 */ { &DRa_4     , 0        , 0      , 0             , 0 },	/* ==>  DRa_4 */
/* 253 */ { 0          , 0        , &l[254], 0             , 0 },	/* ==>  DR@rcl */
/* 254 */ { 0          , 0        , 0      , (char*)&l[249], 0 },	/* ==>  DR@res */
/* 255 */ { 0          , 0        , &l[256], 0             , 0 },	/* ==>  DR@dat */
/* 256 */ { 0          , 0        , &l[257], 0             , 0 },	/* ==>  DR@dcl */
/* 257 */ { 0          , 0        , &l[258], 0             , 0 },	/* ==>  DR@res */
/* 258 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DR@rcl */

/* 259 */ { 0          , 0        , &l[260], 0             , 0 },	/* ==>  DSR@i */
/* 260 */ { 0          , 7        , &l[262], 0             , 0 },	/* ==>  var list */
/* 261 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR */
/* 262 */ { 0          , 0        , &l[263], 0             , 0 },	/* ==>  DSR@ */
/* 263 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  var list */
/* 264 */ { &DSRa_1    , 0        , &l[265], 0             , 0 },	/* ==>  DSRa_1 */
/* 265 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR@res */
/* 266 */ { 0          , 0        , &l[267], 0             , 0 },	/* ==>  DSR@scl */
/* 267 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR@set */
/* 268 */ { &DSRa_2    , 0        , 0      , (char*)&l[260], 0 },	/* ==>  DSRa_2 */
/* 269 */ { &DSRa_3    , 0        , &l[270], 0             , 0 },	/* ==>  DSRa_3 */
/* 270 */ { &DSRa_4    , 0        , &l[271], 0             , 0 },	/* ==>  DSRa_4 */
/* 271 */ { &DSRa_5    , 0        , 0      , 0             , 0 },	/* ==>  DSRa_5 */
/* 272 */ { 0          , 0        , &l[273], 0             , 0 },	/* ==>  DSR@dat */
/* 273 */ { 0          , 1        , 0      , (char*)&l[259], 0 },	/* ==> ~DSR@i */
/* 274 */ { 0          , 0        , &l[275], 0             , 0 },	/* ==>  DSR@ */
/* 275 */ { 0          , 0        , 0      , (char*)&l[273], 0 },	/* ==>  DSR@i */
/* 276 */ { 0          , 0        , &l[277], 0             , 0 },	/* ==>  DSR@dcl */
/* 277 */ { &DSRa_6    , 0        , &l[278], (char*)&l[263], 0 },	/* ==>  DSRa_6 */
/* 278 */ { &DSRa_7    , 0        , 0      , 0             , 0 },	/* ==>  DSRa_7 */
/* 279 */ { 0          , 0        , &l[280], (char*)&l[266], 0 },	/* ==>  DSR@scl */
/* 280 */ { 0          , 0        , &l[281], (char*)&l[267], 0 },	/* ==>  DSR@set */
/* 281 */ { 0          , 1        , 0      , (char*)&l[265], 0 },	/* ==> ~DSR@res */
/* 282 */ { 0          , 0        , &l[283], 0             , 0 },	/* ==>  DSR@rcl */
/* 283 */ { 0          , 1        , &l[284], (char*)&l[280], 0 },	/* ==> ~DSR@set */
/* 284 */ { 0          , 0        , 0      , (char*)&l[281], 0 },	/* ==>  DSR@res */
/* 285 */ { 0          , 0        , &l[286], 0             , 0 },	/* ==>  DSR@dat */
/* 286 */ { 0          , 0        , &l[287], 0             , 0 },	/* ==>  DSR@dcl */
/* 287 */ { 0          , 0        , &l[288], 0             , 0 },	/* ==>  DSR@set */
/* 288 */ { 0          , 0        , &l[289], 0             , 0 },	/* ==>  DSR@scl */
/* 289 */ { 0          , 0        , &l[290], 0             , 0 },	/* ==>  DSR@res */
/* 290 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR@rcl */

/* 291 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR */
/* 292 */ { 0          , 0        , &l[293], 0             , 0 },	/* ==>  SR@ */
/* 293 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 294 */ { &SRa_1     , 0        , &l[295], (char*)&l[293], 0 },	/* ==>  SRa_1 */
/* 295 */ { &SRa_2     , 0        , 0      , 0             , 0 },	/* ==>  SRa_2 */
/* 296 */ { 0          , 0        , &l[297], 0             , 0 },	/* ==>  SR@scl */
/* 297 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR@set */
/* 298 */ { 0          , 0        , &l[299], 0             , 0 },	/* ==>  SR@rcl */
/* 299 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR@res */
/* 300 */ { 0          , 0        , &l[301], 0             , 0 },	/* ==>  SR@set */
/* 301 */ { 0          , 0        , &l[302], 0             , 0 },	/* ==>  SR@scl */
/* 302 */ { 0          , 0        , &l[303], 0             , 0 },	/* ==>  SR@res */
/* 303 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR@rcl */

/* 304 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR */
/* 305 */ { 0          , 0        , &l[306], 0             , 0 },	/* ==>  SRR@ */
/* 306 */ { 0          , 6        , 0      , 0             , 0 },	/* ==>  var list */
/* 307 */ { &SRRa_1    , 0        , &l[308], (char*)&l[306], 0 },	/* ==>  SRRa_1 */
/* 308 */ { &SRRa_2    , 0        , &l[309], 0             , 0 },	/* ==>  SRRa_2 */
/* 309 */ { &SRRa_3    , 0        , 0      , 0             , 0 },	/* ==>  SRRa_3 */
/* 310 */ { 0          , 0        , &l[311], 0             , 0 },	/* ==>  SRR@scl */
/* 311 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR@set */
/* 312 */ { 0          , 0        , &l[313], 0             , 0 },	/* ==>  SRR@rcl */
/* 313 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR@res */
/* 314 */ { 0          , 0        , &l[315], 0             , 0 },	/* ==>  SRR@r2cl */
/* 315 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR@res2 */
/* 316 */ { 0          , 0        , &l[317], 0             , 0 },	/* ==>  SRR@set */
/* 317 */ { 0          , 0        , &l[318], 0             , 0 },	/* ==>  SRR@scl */
/* 318 */ { 0          , 0        , &l[319], 0             , 0 },	/* ==>  SRR@res */
/* 319 */ { 0          , 0        , &l[320], 0             , 0 },	/* ==>  SRR@rcl */
/* 320 */ { 0          , 0        , &l[321], 0             , 0 },	/* ==>  SRR@res2 */
/* 321 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR@r2cl */

/* 322 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRX */
/* 323 */ { 0          , 0        , &l[324], 0             , 0 },	/* ==>  SRX@ */
/* 324 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 325 */ { &SRXa_1    , 0        , &l[326], (char*)&l[324], 0 },	/* ==>  SRXa_1 */
/* 326 */ { &SRXa_2    , 0        , 0      , 0             , 0 },	/* ==>  SRXa_2 */
/* 327 */ { 0          , 0        , &l[328], 0             , 0 },	/* ==>  SRX@scl */
/* 328 */ { 0          , 0        , &l[329], 0             , 0 },	/* ==>  SRX@set */
/* 329 */ { 0          , 1        , 0      , 0             , 0 },	/* ==> ~SRX@res */
/* 330 */ { 0          , 0        , &l[331], 0             , 0 },	/* ==>  SRX@rcl */
/* 331 */ { 0          , 1        , &l[332], (char*)&l[328], 0 },	/* ==> ~SRX@set */
/* 332 */ { 0          , 0        , 0      , (char*)&l[329], 0 },	/* ==>  SRX@res */
/* 333 */ { 0          , 0        , &l[334], 0             , 0 },	/* ==>  SRX@set */
/* 334 */ { 0          , 0        , &l[335], 0             , 0 },	/* ==>  SRX@scl */
/* 335 */ { 0          , 0        , &l[336], 0             , 0 },	/* ==>  SRX@res */
/* 336 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRX@rcl */

/* 337 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  JK */
/* 338 */ { 0          , 0        , &l[339], 0             , 0 },	/* ==>  JK@ */
/* 339 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 340 */ { &JKa_1     , 0        , &l[341], (char*)&l[339], 0 },	/* ==>  JKa_1 */
/* 341 */ { &JKa_2     , 0        , 0      , 0             , 0 },	/* ==>  JKa_2 */
/* 342 */ { 0          , 0        , &l[343], 0             , 0 },	/* ==>  JK@scl */
/* 343 */ { 0          , 1        , &l[344], 0             , 0 },	/* ==> ~JK@ */
/* 344 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  JK@set */
/* 345 */ { 0          , 0        , &l[346], 0             , 0 },	/* ==>  JK@rcl */
/* 346 */ { 0          , 0        , &l[347], (char*)&l[343], 0 },	/* ==>  JK@ */
/* 347 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  JK@res */
/* 348 */ { 0          , 0        , &l[349], 0             , 0 },	/* ==>  JK@set */
/* 349 */ { 0          , 0        , &l[350], 0             , 0 },	/* ==>  JK@scl */
/* 350 */ { 0          , 0        , &l[351], 0             , 0 },	/* ==>  JK@res */
/* 351 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  JK@rcl */

/* 352 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR */
/* 353 */ { 0          , 0        , &l[354], 0             , 0 },	/* ==>  SHR@ */
/* 354 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 355 */ { &SHRa_1    , 0        , &l[356], 0             , 0 },	/* ==>  SHRa_1 */
/* 356 */ { &SHRa_2    , 0        , 0      , 0             , 0 },	/* ==>  SHRa_2 */
/* 357 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR@res */
/* 358 */ { 0          , 0        , &l[359], 0             , 0 },	/* ==>  SHR@dcl */
/* 359 */ { &SHRa_3    , (1<<FO)+1, &l[360], (char*)&l[354], 0 },	/* ==>  SHRa_3 */
/* 360 */ { 0          , (1<<FO)+2, &l[361], 0             , 0 },	/* ==>  SHR@ */
/* 361 */ { 0          , (1<<FO)+3, 0      , 0             , 0 },	/* ==>  SHR@dat */
/* 362 */ { 0          , 0        , &l[363], 0             , 0 },	/* ==>  SHR@rcl */
/* 363 */ { 0          , 0        , 0      , (char*)&l[357], 0 },	/* ==>  SHR@res */
/* 364 */ { 0          , 0        , &l[365], 0             , 0 },	/* ==>  SHR@dat */
/* 365 */ { 0          , 0        , &l[366], 0             , 0 },	/* ==>  SHR@dcl */
/* 366 */ { 0          , 0        , &l[367], 0             , 0 },	/* ==>  SHR@res */
/* 367 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR@rcl */

/* 368 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR */
/* 369 */ { 0          , 0        , &l[370], 0             , 0 },	/* ==>  SHSR@ */
/* 370 */ { 0          , 6        , 0      , 0             , 0 },	/* ==>  var list */
/* 371 */ { &SHSRa_1   , 0        , &l[372], 0             , 0 },	/* ==>  SHSRa_1 */
/* 372 */ { &SHSRa_2   , 0        , &l[373], 0             , 0 },	/* ==>  SHSRa_2 */
/* 373 */ { &SHSRa_3   , 0        , 0      , 0             , 0 },	/* ==>  SHSRa_3 */
/* 374 */ { 0          , 0        , &l[375], 0             , 0 },	/* ==>  SHSR@set */
/* 375 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR@res */
/* 376 */ { 0          , 0        , &l[377], 0             , 0 },	/* ==>  SHSR@dcl */
/* 377 */ { &SHSRa_4   , (1<<FO)+1, &l[378], (char*)&l[370], 0 },	/* ==>  SHSRa_4 */
/* 378 */ { 0          , (1<<FO)+2, &l[379], 0             , 0 },	/* ==>  SHSR@ */
/* 379 */ { 0          , (1<<FO)+3, 0      , 0             , 0 },	/* ==>  SHSR@dat */
/* 380 */ { 0          , 0        , &l[381], 0             , 0 },	/* ==>  SHSR@scl */
/* 381 */ { 0          , 0        , &l[382], (char*)&l[374], 0 },	/* ==>  SHSR@set */
/* 382 */ { 0          , 1        , 0      , (char*)&l[375], 0 },	/* ==> ~SHSR@res */
/* 383 */ { 0          , 0        , &l[384], 0             , 0 },	/* ==>  SHSR@rcl */
/* 384 */ { 0          , 1        , &l[385], (char*)&l[381], 0 },	/* ==> ~SHSR@set */
/* 385 */ { 0          , 0        , 0      , (char*)&l[382], 0 },	/* ==>  SHSR@res */
/* 386 */ { 0          , 0        , &l[387], 0             , 0 },	/* ==>  SHSR@dat */
/* 387 */ { 0          , 0        , &l[388], 0             , 0 },	/* ==>  SHSR@dcl */
/* 388 */ { 0          , 0        , &l[389], 0             , 0 },	/* ==>  SHSR@set */
/* 389 */ { 0          , 0        , &l[390], 0             , 0 },	/* ==>  SHSR@scl */
/* 390 */ { 0          , 0        , &l[391], 0             , 0 },	/* ==>  SHSR@res */
/* 391 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR@rcl */

/* 392 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  ST */
/* 393 */ { 0          , 0        , &l[394], 0             , 0 },	/* ==>  ST@ */
/* 394 */ { 0          , 3        , 0      , 0             , 0 },	/* ==>  var list */
/* 395 */ { &STa_1     , 0        , &l[396], (char*)&l[394], 0 },	/* ==>  STa_1 */
/* 396 */ { &STa_2     , 0        , 0      , 0             , 0 },	/* ==>  STa_2 */
/* 397 */ { 0          , 0        , &l[398], 0             , 0 },	/* ==>  ST@scl */
/* 398 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  ST@set */
/* 399 */ { 0          , 0        , &l[400], 0             , 0 },	/* ==>  ST@tcl */
/* 400 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  ST@ */
/* 401 */ { 0          , 0        , &l[402], 0             , 0 },	/* ==>  ST@set */
/* 402 */ { 0          , 0        , &l[403], 0             , 0 },	/* ==>  ST@scl */
/* 403 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  ST@tcl */

/* 404 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRT */
/* 405 */ { 0          , 0        , &l[406], 0             , 0 },	/* ==>  SRT@ */
/* 406 */ { 0          , 5        , 0      , 0             , 0 },	/* ==>  var list */
/* 407 */ { &SRTa_1    , 0        , &l[408], (char*)&l[406], 0 },	/* ==>  SRTa_1 */
/* 408 */ { &SRTa_2    , 0        , &l[409], 0             , 0 },	/* ==>  SRTa_2 */
/* 409 */ { &SRTa_3    , 0        , 0      , 0             , 0 },	/* ==>  SRTa_3 */
/* 410 */ { 0          , 0        , &l[411], 0             , 0 },	/* ==>  SRT@scl */
/* 411 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRT@set */
/* 412 */ { 0          , 0        , &l[413], 0             , 0 },	/* ==>  SRT@rcl */
/* 413 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRT@res */
/* 414 */ { 0          , 0        , &l[415], 0             , 0 },	/* ==>  SRT@tcl */
/* 415 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRT@ */
/* 416 */ { 0          , 0        , &l[417], 0             , 0 },	/* ==>  SRT@set */
/* 417 */ { 0          , 0        , &l[418], 0             , 0 },	/* ==>  SRT@scl */
/* 418 */ { 0          , 0        , &l[419], 0             , 0 },	/* ==>  SRT@res */
/* 419 */ { 0          , 0        , &l[420], 0             , 0 },	/* ==>  SRT@rcl */
/* 420 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRT@tcl */

/* 421 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  FALL */
/* 422 */ { 0          , 0        , &l[423], 0             , 0 },	/* ==>  FALL@ */
/* 423 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 424 */ { &FALLa_1   , 0        , 0      , (char*)&l[423], 0 },	/* ==>  FALLa_1 */
/* 425 */ { 0          , 0        , &l[426], 0             , 0 },	/* ==>  FALL@clk */
/* 426 */ { 0          , 1        , 0      , 0             , 0 },	/* ==> ~FALL@fall */
/* 427 */ { 0          , 0        , &l[428], 0             , 0 },	/* ==>  FALL@fall */
/* 428 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  FALL@clk */

/* 429 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  LATCH */
/* 430 */ { 0          , 0        , &l[431], 0             , 0 },	/* ==>  LATCH@ */
/* 431 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  no var list */
/* 432 */ { 0          , 0        , &l[433], 0             , 0 },	/* ==>  LATCH@ */
/* 433 */ { 0          , 0        , &l[434], 0             , 0 },	/* ==>  LATCH@set */
/* 434 */ { 0          , 1        , 0      , 0             , 0 },	/* ==> ~LATCH@res */
/* 435 */ { 0          , 0        , &l[436], 0             , 0 },	/* ==>  LATCH@set */
/* 436 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  LATCH@res */

/* 437 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DLATCH */
/* 438 */ { 0          , 0        , &l[439], 0             , 0 },	/* ==>  DLATCH@ */
/* 439 */ { 0          , 3        , 0      , 0             , 0 },	/* ==>  var list */
/* 440 */ { &DLATCHa_1 , 0        , 0      , (char*)&l[439], 0 },	/* ==>  DLATCHa_1 */
/* 441 */ { 0          , 0        , &l[442], 0             , 0 },	/* ==>  DLATCH@clk */
/* 442 */ { 0          , 0        , &l[443], 0             , 0 },	/* ==>  DLATCH@ */
/* 443 */ { 0          , 0        , &l[444], 0             , 0 },	/* ==>  DLATCH@set */
/* 444 */ { 0          , 1        , 0      , 0             , 0 },	/* ==> ~DLATCH@res */
/* 445 */ { 0          , 0        , &l[446], 0             , 0 },	/* ==>  DLATCH@set */
/* 446 */ { 0          , 0        , &l[447], 0             , 0 },	/* ==>  DLATCH@res */
/* 447 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DLATCH@clk */
}; /* l[] */
#else	/* BOOT_COMPILE */
Symbol		iC_CHANGE_ar = { "CHANGE", KEYW, CH_AR, };	/* alternative arithmetic CHANGE */
#endif	/* BOOT_COMPILE */

/********************************************************************
 *
 *	Array b[] for setting up initial symbols in the Symbol Table for
 *	pre-compiled built in function blocks, iC and C reserved words as
 *	well as tokens for compiling function blocks in BOOT_COMPILE mode
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
 *					ftype	used in iC compilation
 *					uVal	yacc compiler_token
 *
 *******************************************************************/

static BuiltIn b[] = {
#ifndef BOOT_COMPILE
/*  "name"           type   ftype  fm    list     blist    link1    link2/glist     */

  { "FORCE",       IFUNCT,  GATE,  0x3f, &l[6]  , &l[1]  , &l[0]  , 0               },	/* 0 pre-installed "FORCE" function block */
  { "FORCE@",        LATCH, GATE,  0x8d, &l[0]  , &l[3]  , &l[1]  , 0               },
  { "FORCE@arg1",    OR,    GATE,  0x85, 0      , 0      , &l[3]  , &l[6]           },
  { "FORCE@on",      OR,    GATE,  0x85, 0      , 0      , &l[4]  , &l[7]           },
  { "FORCE@off",     OR,    GATE,  0x85, 0      , 0      , &l[5]  , &l[8]           },

  { "D",           IFUNCT,  GATE,  0x03, &l[15] , &l[10] , &l[9]  , 0               },	/* 5 pre-installed "D" function block */
  { "D@",            FF,    GATE,  0x8d, &l[9]  , &l[12] , &l[10] , 0               },
  { "D@expr",        OR,    GATE,  0x85, 0      , 0      , &l[14] , &l[15]          },
  { "D@dcl",         CLK,   CLCKL, 0x85, 0      , 0      , &l[13] , &l[16]          },

  { "DS_",         IFUNCT,  GATE,  0x0f, &l[26] , &l[18] , &l[17] , 0               },	/* 9 pre-installed "DS_" function block */
  { "DS_@",          FF,    GATE,  0x8d, &l[17] , &l[20] , &l[18] , 0               },
  { "DS_@expr",      OR,    GATE,  0x85, 0      , 0      , &l[23] , &l[26]          },
  { "DS_@dcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[22] , &l[27]          },
  { "DS_@set",       OR,    GATE,  0x85, 0      , 0      , &l[25] , &l[28]          },
  { "DS_@scl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[24] , &l[29]          },

  { "DR_",         IFUNCT,  GATE,  0x0f, &l[39] , &l[31] , &l[30] , 0               },	/* 15 pre-installed "DR_" function block */
  { "DR_@",          FF,    GATE,  0x8d, &l[30] , &l[33] , &l[31] , 0               },
  { "DR_@expr",      OR,    GATE,  0x85, 0      , 0      , &l[36] , &l[39]          },
  { "DR_@dcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[35] , &l[40]          },
  { "DR_@res",       OR,    GATE,  0x85, 0      , 0      , &l[38] , &l[41]          },
  { "DR_@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[37] , &l[42]          },

  { "DSR_",        IFUNCT,  GATE,  0x3f, &l[55] , &l[44] , &l[43] , 0               },	/* 21 pre-installed "DSR_" function block */
  { "DSR_@",         FF,    GATE,  0x8d, &l[43] , &l[46] , &l[44] , 0               },
  { "DSR_@expr",     OR,    GATE,  0x85, 0      , 0      , &l[50] , &l[55]          },
  { "DSR_@dcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[49] , &l[56]          },
  { "DSR_@set",      OR,    GATE,  0x85, 0      , 0      , &l[52] , &l[57]          },
  { "DSR_@scl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[51] , &l[58]          },
  { "DSR_@res",      OR,    GATE,  0x85, 0      , 0      , &l[54] , &l[59]          },
  { "DSR_@rcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[53] , &l[60]          },

  { "SR_",         IFUNCT,  GATE,  0x0f, &l[70] , &l[62] , &l[61] , 0               },	/* 29 pre-installed "SR_" function block */
  { "SR_@",          FF,    GATE,  0x8d, &l[61] , &l[64] , &l[62] , 0               },
  { "SR_@set",       OR,    GATE,  0x85, 0      , 0      , &l[67] , &l[70]          },
  { "SR_@scl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[66] , &l[71]          },
  { "SR_@res",       OR,    GATE,  0x85, 0      , 0      , &l[69] , &l[72]          },
  { "SR_@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[68] , &l[73]          },

  { "SRR_",        IFUNCT,  GATE,  0x3f, &l[86] , &l[75] , &l[74] , 0               },	/* 35 pre-installed "SRR_" function block */
  { "SRR_@",         FF,    GATE,  0x8d, &l[74] , &l[77] , &l[75] , 0               },
  { "SRR_@set",      OR,    GATE,  0x85, 0      , 0      , &l[81] , &l[86]          },
  { "SRR_@scl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[80] , &l[87]          },
  { "SRR_@res",      OR,    GATE,  0x85, 0      , 0      , &l[83] , &l[88]          },
  { "SRR_@rcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[82] , &l[89]          },
  { "SRR_@res2",     OR,    GATE,  0x85, 0      , 0      , &l[85] , &l[90]          },
  { "SRR_@r2cl",     CLK,   CLCKL, 0x85, 0      , 0      , &l[84] , &l[91]          },

  { "SH",          IFUNCT,  ARITH, 0x03, &l[98] , &l[93] , &l[92] , 0               },	/* 43 pre-installed "SH" function block */
  { "SH@",           SH,    ARITH, 0x8d, &l[92] , &l[95] , &l[93] , 0               },
  { "SH@dat",        ARN,   ARITH, 0x85, 0      , 0      , &l[97] , &l[98]          },
  { "SH@dcl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[96] , &l[99]          },

  { "SHR_",        IFUNCT,  ARITH, 0x0f, &l[109], &l[101], &l[100], 0               },	/* 47 pre-installed "SHR_" function block */
  { "SHR_@",         SH,    ARITH, 0x8d, &l[100], &l[103], &l[101], 0               },
  { "SHR_@dat",      ARN,   ARITH, 0x85, 0      , 0      , &l[106], &l[109]         },
  { "SHR_@dcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[105], &l[110]         },
  { "SHR_@res",      OR,    GATE,  0x85, 0      , 0      , &l[108], &l[111]         },
  { "SHR_@rcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[107], &l[112]         },

  { "SHSR_",       IFUNCT,  ARITH, 0x3f, &l[125], &l[114], &l[113], 0               },	/* 53 pre-installed "SHSR_" function block */
  { "SHSR_@",        SH,    ARITH, 0x8d, &l[113], &l[116], &l[114], 0               },
  { "SHSR_@dat",     ARN,   ARITH, 0x85, 0      , 0      , &l[120], &l[125]         },
  { "SHSR_@dcl",     CLK,   CLCKL, 0x85, 0      , 0      , &l[119], &l[126]         },
  { "SHSR_@set",     OR,    GATE,  0x85, 0      , 0      , &l[122], &l[127]         },
  { "SHSR_@scl",     CLK,   CLCKL, 0x85, 0      , 0      , &l[121], &l[128]         },
  { "SHSR_@res",     OR,    GATE,  0x85, 0      , 0      , &l[124], &l[129]         },
  { "SHSR_@rcl",     CLK,   CLCKL, 0x85, 0      , 0      , &l[123], &l[130]         },

  { "RISE",        IFUNCT,  GATE,  0x03, &l[137], &l[132], &l[131], 0               },	/* 61 pre-installed "RISE" function block */
  { "RISE@",         EF,    GATE,  0x8d, &l[131], &l[134], &l[132], 0               },
  { "RISE@trig",     OR,    GATE,  0x85, 0      , 0      , &l[136], &l[137]         },
  { "RISE@clk",      CLK,   CLCKL, 0x85, 0      , 0      , &l[135], &l[138]         },

  { "CHANGE",      IFUNCT,  GATE,  0x01, &l[145], &l[140], &l[139], 0               },	/* 65 pre-installed "CHANGE" function block */
  { "CHANGE@",       VF,    GATE,  0x8d, &l[139], &l[142], &l[140], 0               },
  { "CHANGE@trig",   OR,    GATE,  0x85, 0      , 0      , &l[144], &l[145]         },
  { "CHANGE@clk",    CLK,   CLCKL, 0x85, 0      , 0      , &l[143], &l[146]         },

  { "CHANGE2",     IFUNCT,  GATE,  0x02, &l[153], &l[148], &l[147], (List_e*)&b[65] },	/* 69 pre-installed "CHANGE2" alternate "CHANGE" */
  { "CHANGE2@",      VF,    GATE,  0x8d, &l[147], &l[150], &l[148], 0               },
  { "CHANGE2@val",   ARN,   ARITH, 0x85, 0      , 0      , &l[152], &l[153]         },
  { "CHANGE2@clk",   CLK,   CLCKL, 0x85, 0      , 0      , &l[151], &l[154]         },

  { "CLOCK",       IFUNCT,  CLCKL, 0x03, &l[161], &l[156], &l[155], 0               },	/* 73 pre-installed "CLOCK" function block */
  { "CLOCK@",        CLK,   CLCKL, 0x8d, &l[155], &l[158], &l[156], 0               },
  { "CLOCK@trig",    OR,    GATE,  0x85, 0      , 0      , &l[160], &l[161]         },
  { "CLOCK@clk",     CLK,   CLCKL, 0x85, 0      , 0      , &l[159], &l[162]         },

  { "CLOCK2",      IFUNCT,  CLCKL, 0x0f, &l[172], &l[164], &l[163], (List_e*)&b[73] },	/* 77 pre-installed "CLOCK2" alternate "CLOCK" */
  { "CLOCK2@",       CLK,   CLCKL, 0x8d, &l[163], &l[166], &l[164], 0               },
  { "CLOCK2@trig",   OR,    GATE,  0x85, 0      , 0      , &l[169], &l[172]         },
  { "CLOCK2@clk",    CLK,   CLCKL, 0x85, 0      , 0      , &l[168], &l[173]         },
  { "CLOCK2@trig2",  OR,    GATE,  0x85, 0      , 0      , &l[171], &l[174]         },
  { "CLOCK2@clk2",   CLK,   CLCKL, 0x85, 0      , 0      , &l[170], &l[175]         },

  { "TIMER",       IFUNCT,  TIMRL, 0x03, &l[182], &l[177], &l[176], 0               },	/* 83 pre-installed "TIMER" function block */
  { "TIMER@",        TIM,   TIMRL, 0x8d, &l[176], &l[179], &l[177], 0               },
  { "TIMER@trig",    OR,    GATE,  0x85, 0      , 0      , &l[181], &l[182]         },
  { "TIMER@clk",     CLK,   CLCKL, 0x85, 0      , 0      , &l[180], &l[183]         },

  { "TIMER02",     IFUNCT,  TIMRL, 0x0f, &l[193], &l[185], &l[184], (List_e*)&b[83] },	/* 87 pre-installed "TIMER02" alternate "TIMER" */
  { "TIMER02@",      TIM,   TIMRL, 0x8d, &l[184], &l[187], &l[185], 0               },
  { "TIMER02@trig",  OR,    GATE,  0x85, 0      , 0      , &l[190], &l[193]         },
  { "TIMER02@clk",   CLK,   CLCKL, 0x85, 0      , 0      , &l[189], &l[194]         },
  { "TIMER02@trig2", OR,    GATE,  0x85, 0      , 0      , &l[192], &l[195]         },
  { "TIMER02@clk2",  CLK,   CLCKL, 0x85, 0      , 0      , &l[191], &l[196]         },

  { "TIMER1",      IFUNCT,  TIMRL, 0x03, &l[203], &l[198], &l[197], 0               },	/* 93 pre-installed "TIMER1" function block */
  { "TIMER1@",       TIM,   TIMRL, 0x8d, &l[197], &l[200], &l[198], 0               },
  { "TIMER1@trig",   OR,    GATE,  0x85, 0      , 0      , &l[202], &l[203]         },
  { "TIMER1@clk",    CLK,   CLCKL, 0x85, 0      , 0      , &l[201], &l[204]         },

  { "TIMER12",     IFUNCT,  TIMRL, 0x0f, &l[214], &l[206], &l[205], (List_e*)&b[93] },	/* 97 pre-installed "TIMER12" alternate "TIMER1" */
  { "TIMER12@",      TIM,   TIMRL, 0x8d, &l[205], &l[208], &l[206], 0               },
  { "TIMER12@trig",  OR,    GATE,  0x85, 0      , 0      , &l[211], &l[214]         },
  { "TIMER12@clk",   CLK,   CLCKL, 0x85, 0      , 0      , &l[210], &l[215]         },
  { "TIMER12@trig2", OR,    GATE,  0x85, 0      , 0      , &l[213], &l[216]         },
  { "TIMER12@clk2",  CLK,   CLCKL, 0x85, 0      , 0      , &l[212], &l[217]         },

  { "DS@i",          FF,    GATE,  0x8a, 0      , &l[223], &l[231], 0               },
  { "DS",          IFUNCT,  GATE,  0x0f, &l[237], &l[218], &l[220], 0               },	/* 104 pre-installed "DS" function block */
  { "DS@",           FF,    GATE,  0x8e, &l[220], &l[226], &l[221], &l[230]         },
  { "DS@dat",        OR,    GATE,  0x85, 0      , 0      , &l[228], &l[237]         },
  { "DS@dcl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[232], &l[238]         },
  { "DS@set",        OR,    GATE,  0x86, 0      , 0      , &l[236], &l[239]         },
  { "DS@scl",        CLK,   CLCKL, 0x86, 0      , 0      , &l[235], &l[240]         },

  { "DR",          IFUNCT,  GATE,  0x0f, &l[255], &l[242], &l[241], 0               },	/* 110 pre-installed "DR" function block */
  { "DR@",           FF,    GATE,  0x8e, &l[241], &l[244], &l[242], &l[248]         },
  { "DR@dat",        OR,    GATE,  0x85, 0      , 0      , &l[246], &l[255]         },
  { "DR@dcl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[250], &l[256]         },
  { "DR@res",        OR,    GATE,  0x86, 0      , 0      , &l[254], &l[257]         },
  { "DR@rcl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[253], &l[258]         },

  { "DSR@i",         OR,    GATE,  0x8a, 0      , &l[264], &l[275], 0               },
  { "DSR",         IFUNCT,  GATE,  0x3f, &l[285], &l[259], &l[261], 0               },	/* 117 pre-installed "DSR" function block */
  { "DSR@",          FF,    GATE,  0x8e, &l[261], &l[269], &l[262], &l[274]         },
  { "DSR@dat",       OR,    GATE,  0x85, 0      , 0      , &l[272], &l[285]         },
  { "DSR@dcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[276], &l[286]         },
  { "DSR@set",       OR,    GATE,  0x86, 0      , 0      , &l[283], &l[287]         },
  { "DSR@scl",       CLK,   CLCKL, 0x86, 0      , 0      , &l[279], &l[288]         },
  { "DSR@res",       OR,    GATE,  0x86, 0      , 0      , &l[284], &l[289]         },
  { "DSR@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[282], &l[290]         },

  { "SR",          IFUNCT,  GATE,  0x0f, &l[300], &l[292], &l[291], 0               },	/* 125 pre-installed "SR" function block */
  { "SR@",           FF,    GATE,  0x8d, &l[291], &l[294], &l[292], 0               },
  { "SR@set",        OR,    GATE,  0x85, 0      , 0      , &l[297], &l[300]         },
  { "SR@scl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[296], &l[301]         },
  { "SR@res",        OR,    GATE,  0x85, 0      , 0      , &l[299], &l[302]         },
  { "SR@rcl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[298], &l[303]         },

  { "SRR",         IFUNCT,  GATE,  0x3f, &l[316], &l[305], &l[304], 0               },	/* 131 pre-installed "SRR" function block */
  { "SRR@",          FF,    GATE,  0x8d, &l[304], &l[307], &l[305], 0               },
  { "SRR@set",       OR,    GATE,  0x85, 0      , 0      , &l[311], &l[316]         },
  { "SRR@scl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[310], &l[317]         },
  { "SRR@res",       OR,    GATE,  0x85, 0      , 0      , &l[313], &l[318]         },
  { "SRR@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[312], &l[319]         },
  { "SRR@res2",      OR,    GATE,  0x85, 0      , 0      , &l[315], &l[320]         },
  { "SRR@r2cl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[314], &l[321]         },

  { "SRX",         IFUNCT,  GATE,  0x0f, &l[333], &l[323], &l[322], 0               },	/* 139 pre-installed "SRX" function block */
  { "SRX@",          FF,    GATE,  0x8d, &l[322], &l[325], &l[323], 0               },
  { "SRX@set",       OR,    GATE,  0x86, 0      , 0      , &l[331], &l[333]         },
  { "SRX@scl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[327], &l[334]         },
  { "SRX@res",       OR,    GATE,  0x86, 0      , 0      , &l[332], &l[335]         },
  { "SRX@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[330], &l[336]         },

  { "JK",          IFUNCT,  GATE,  0x0f, &l[348], &l[338], &l[337], 0               },	/* 145 pre-installed "JK" function block */
  { "JK@",           FF,    GATE,  0x8e, &l[337], &l[340], &l[338], &l[346]         },
  { "JK@set",        OR,    GATE,  0x85, 0      , 0      , &l[344], &l[348]         },
  { "JK@scl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[342], &l[349]         },
  { "JK@res",        OR,    GATE,  0x85, 0      , 0      , &l[347], &l[350]         },
  { "JK@rcl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[345], &l[351]         },

  { "SHR",         IFUNCT,  ARITH, 0x0f, &l[364], &l[353], &l[352], 0               },	/* 151 pre-installed "SHR" function block */
  { "SHR@",          SH,    ARITH, 0x8e, &l[352], &l[355], &l[353], &l[360]         },
  { "SHR@dat",       ARN,   ARITH, 0x85, 0      , 0      , &l[361], &l[364]         },
  { "SHR@dcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[358], &l[365]         },
  { "SHR@res",       OR,    GATE,  0x86, 0      , 0      , &l[363], &l[366]         },
  { "SHR@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[362], &l[367]         },

  { "SHSR",        IFUNCT,  ARITH, 0x3f, &l[386], &l[369], &l[368], 0               },	/* 157 pre-installed "SHSR" function block */
  { "SHSR@",         SH,    ARITH, 0x8e, &l[368], &l[371], &l[369], &l[378]         },
  { "SHSR@dat",      ARN,   ARITH, 0x85, 0      , 0      , &l[379], &l[386]         },
  { "SHSR@dcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[376], &l[387]         },
  { "SHSR@set",      OR,    GATE,  0x86, 0      , 0      , &l[384], &l[388]         },
  { "SHSR@scl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[380], &l[389]         },
  { "SHSR@res",      OR,    GATE,  0x86, 0      , 0      , &l[385], &l[390]         },
  { "SHSR@rcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[383], &l[391]         },

  { "ST",          IFUNCT,  GATE,  0x03, &l[401], &l[393], &l[392], 0               },	/* 165 pre-installed "ST" function block */
  { "ST@",           FF,    GATE,  0x8e, &l[392], &l[395], &l[393], &l[400]         },
  { "ST@set",        OR,    GATE,  0x85, 0      , 0      , &l[398], &l[401]         },
  { "ST@scl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[397], &l[402]         },
  { "ST@tcl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[399], &l[403]         },

  { "SRT",         IFUNCT,  GATE,  0x0f, &l[416], &l[405], &l[404], 0               },	/* 170 pre-installed "SRT" function block */
  { "SRT@",          FF,    GATE,  0x8e, &l[404], &l[407], &l[405], &l[415]         },
  { "SRT@set",       OR,    GATE,  0x85, 0      , 0      , &l[411], &l[416]         },
  { "SRT@scl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[410], &l[417]         },
  { "SRT@res",       OR,    GATE,  0x85, 0      , 0      , &l[413], &l[418]         },
  { "SRT@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[412], &l[419]         },
  { "SRT@tcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[414], &l[420]         },

  { "FALL",        IFUNCT,  GATE,  0x03, &l[427], &l[422], &l[421], 0               },	/* 177 pre-installed "FALL" function block */
  { "FALL@",         EF,    GATE,  0x8d, &l[421], &l[424], &l[422], 0               },
  { "FALL@fall",     OR,    GATE,  0x85, 0      , 0      , &l[426], &l[427]         },
  { "FALL@clk",      CLK,   CLCKL, 0x85, 0      , 0      , &l[425], &l[428]         },

  { "LATCH",       IFUNCT,  GATE,  0x0f, &l[435], &l[430], &l[429], 0               },	/* 181 pre-installed "LATCH" function block */
  { "LATCH@",        LATCH, GATE,  0x8e, &l[429], &l[432], &l[430], &l[432]         },
  { "LATCH@set",     OR,    GATE,  0x85, 0      , 0      , &l[433], &l[435]         },
  { "LATCH@res",     OR,    GATE,  0x85, 0      , 0      , &l[434], &l[436]         },

  { "DLATCH",      IFUNCT,  GATE,  0x0f, &l[445], &l[438], &l[437], 0               },	/* 185 pre-installed "DLATCH" function block */
  { "DLATCH@",       FF,    GATE,  0x8e, &l[437], &l[440], &l[438], &l[442]         },
  { "DLATCH@set",    OR,    GATE,  0x85, 0      , 0      , &l[443], &l[445]         },
  { "DLATCH@res",    OR,    GATE,  0x85, 0      , 0      , &l[444], &l[446]         },
  { "DLATCH@clk",    CLK,   CLCKL, 0x85, 0      , 0      , &l[441], &l[447]         },

											/* 190 pre-installed BuiltIn function Symbols */
#else	/* BOOT_COMPILE */
  /* name	     type   ftype  uVal	*/
  { "FORCE",	     KEYW,  0,     BFORCE, 0    , 0      , 0      , 0 },	/* FORCE function - generates LATCH */
  { "D",	     KEYW,  D_FF,  BLTIN1, 0    , 0      , 0      , 0 },	/* D flip-flop */
  { "DS_",	     KEYW,  D_FF,  BLTINS, 0    , 0      , 0      , 0 },	/* D flip-flop with simple set */
  { "DR_",	     KEYW,  D_FF,  BLTIN2, 0    , 0      , 0      , 0 },	/* D flip-flop with simple reset */
  { "DSR_",	     KEYW,  D_FF,  BLTIN3, 0    , 0      , 0      , 0 },	/* D flip-flop with simple set/reset */
  { "SR_",	     KEYW,  S_FF,  BLTIN2, 0    , 0      , 0      , 0 },	/* SR flip-flop with simple set/reset */
  { "SRR_",	     KEYW,  S_FF,  BLTIN3, 0    , 0      , 0      , 0 },	/* SRR flip-flop with simple set/2xreset */
  { "SH",	     KEYW,  D_SH,  BLTIN1, 0    , 0      , 0      , 0 },	/* arithmetic sample and hold */
  { "SHR_",	     KEYW,  D_SH,  BLTIN2, 0    , 0      , 0      , 0 },	/* sample and hold with simple reset */
  { "SHSR_",	     KEYW,  D_SH,  BLTIN3, 0    , 0      , 0      , 0 },	/* sample and hold with simple set/reset */
  { "RISE",	     KEYW,  RI_BIT,BLTIN1, 0    , 0      , 0      , 0 },	/* pulse on digital rising edge */
  { "CHANGE",	     KEYW,  CH_BIT,BLTINC, 0    , 0      , 0      , 0 },	/* pulse on digital (CH_BIT) or analog (CH_AR) change */
  { "CLOCK",	     KEYW,  CLCK,  CBLTIN, 0    , 0      , 0      , 0 },	/* clock driver */
  { "TIMER",	     KEYW,  TIMR,  TBLTIN, 0    , 0      , 0      , 0 },	/* normal timer driver with preset off 0 */
  { "TIMER1",	     KEYW,  TIMR,  TBLTI1, 0    , 0      , 0      , 0 },	/* alternate timer driver with preset off 1 */
#endif	/* BOOT_COMPILE */
  { "if",	     KEYW,  F_CF,  IF,     0    , 0      , 0      , 0 },
  { "else",	     KEYW,  F_CE,  ELSE,   0    , 0      , 0      , 0 },
  { "switch",	     KEYW,  F_SW,  SWITCH, 0    , 0      , 0      , 0 },
  { "extern",	     KEYW,  0,     EXTERN, 0    , 0      , 0      , 0 },
  { "assign",	     KEYW,  0,     ASSIGN, 0    , 0      , 0      , 0 },
  { "return",	     KEYW,  0,     RETURN, 0    , 0      , 0      , 0 },
  { "no",	     KEYW,  0,     USE,    0    , 0      , 0      , 0 },	/* turn off PRAGMA */
  { "use",	     KEYW,  1,     USE,    0    , 0      , 0      , 0 },	/* turn on PRAGMA */
  { "alias",	     KEYW,  0,     USETYPE,0    , 0      , 0      , 0 },	/* PRAGMA - check that USETYPE < MAXUSETYPE */
  { "strict",	     KEYW,  1,     USETYPE,0    , 0      , 0      , 0 },
  { "list",	     KEYW,  2,     USETYPE,0    , 0      , 0      , 0 },	/* PRAGMA - MAXUSETYPE 3 */
  { "imm",	     KEYW,  0,     IMM,    0    , 0      , 0      , 0 },
  { "immC",	     KEYW,  1,     IMMC,   0    , 0      , 0      , 0 },
  { "void",	     KEYW,  UDFA,  VOID,   0    , 0      , 0      , 0 },
  { "bit",	     KEYW,  GATE,  TYPE,   0    , 0      , 0      , 0 },
  { "int",	     KEYW,  ARITH, TYPE,   0    , 0      , 0      , 0 },
  { "const",	     KEYW,  ARITH, CONST,  0    , 0      , 0      , 0 },	/* used as function block formal parameter */
  { "clock",	     KEYW,  CLCKL, TYPE,   0    , 0      , 0      , 0 },
  { "timer",	     KEYW,  TIMRL, TYPE,   0    , 0      , 0      , 0 },
  { "sizeof",	     KEYW,  0,     SIZEOF, 0    , 0      , 0      , 0 },
  { "this",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },	/* only used in function block definitions */
  { "auto",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },	/* C keywords to cause syntax errors if used in iC */
  { "break",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "case",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "char",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "continue",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "default",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "do",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "double",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "enum",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "float",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "for",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "goto",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "long",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "register",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "short",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "signed",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "static",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "struct",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "typedef",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "union",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "unsigned",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "volatile",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "while",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "fortran",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "asm",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },
  { "FOR",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },	/* keyword used in immac FOR loops */
  { "IF",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },	/* keyword used in immac IF statements */
  { "ELSE",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },	/* keyword used in immac ELSE statements */
  { "ELSIF",	     KEYW,  0,     LEXERR, 0    , 0      , 0      , 0 },	/* keyword used in immac ELSIF statements */
  { "iC_Gt",	     CTYPE, 0,   YYERRCODE,0    , 0      , 0      , 0 },	/* initial Gate C-type from icg.h */
  { "iClock",	     CLK,   CLCKL, 0,      0    , 0      , 0      , 0 },	/* Symbol "iClock" must be last non-zero entry */
  { 0,  	     0,     0,     0,      0    , 0      , 0      , 0 },	/* terminate initialisation of S.T. with name == 0 */
}; /* b[] */
					/* ### end    generated data here ### */

/********************************************************************
 *
 *	Install reserved words and pre-compiled built-ins
 *	or yacc tokens for old style built-ins if BOOT_COMPILE defined
 *
 *******************************************************************/

void
iC_init(void)
{
    int		io;
    int		num;
    Symbol *	sp;
    FILE *	H1p;
    char	lineBuf[BUFS];
    char	tag[BUFS];
    char	ids[BUFS];
#ifndef BOOT_COMPILE
    int		vFlag;
    int		c;
    Symbol *	tsp;
    List_e *	lp;
    List_e *	lp1 = NULL;
    char *	ca;
    char *	cp;
#endif	/* BOOT_COMPILE */

    /********************************************************************
     *  Install pre-defined Symbols in S.T.
     *******************************************************************/
    for (io = 0; b[io].name; io++) {
	/* no name may occur twice - otherwise install fails */
	sp = install(b[io].name, b[io].type, b[io].ftype);
#ifndef BOOT_COMPILE
	if ((lp = b[io].link1) == 0) {		/* reserved word */
#endif	/* BOOT_COMPILE */
	    sp->u_val = b[io].uVal;		/* set u_val in Symbol just installed */
#ifndef BOOT_COMPILE
	} else {
	    /********************************************************************
	     *  Symbol for a pre-compiled built in function block
	     *******************************************************************/
	    sp->fm = b[io].uVal;		/* set fm in Symbol just installed */
	    sp->list = b[io].list;		/* set list in Symbol just installed */
	    sp->u_blist = b[io].blist;		/* set u_blist in Symbol just installed */
	    assert(lp->le_sym == NULL);
	    lp->le_sym = sp;			/* 1st back link to complete built-in net */
	}
	if ((lp = b[io].link2) != 0) {
	    if (sp->type == IFUNCT) {
		tsp = lookup(((BuiltIn*)lp)->name);
		assert(tsp->v_glist == 0);
		tsp->v_glist = sp;		/* forward Symbol link to alternate IFUNCT */
	    } else {
		assert(lp->le_sym == NULL);
		lp->le_sym = sp;		/* parameter back link to complete built-in net */
		if (lp > lp1) {
		    lp1 = lp;			/* last link in l[] for reverse scan */
		}
	    }
	}
#endif	/* BOOT_COMPILE */
    }
    iclock = sp;	/* 'iclock' with name "iClock" is default clock when loop finishes */
#ifndef BOOT_COMPILE
    /********************************************************************
     *  Reverse scan l[] to complete built-in net including static Symbols
     *******************************************************************/
    for (lp = lp1; lp >= l; lp--) {
	if ((lp1 = (List_e*)lp->le_first) != 0) {
	    assert(lp->le_sym);			/* reverse scan ensures Symbol entries are set */
	    lp1->le_sym = lp->le_sym;		/* further back links to complete built-in net */
	    lp->le_first = 0;			/* not used as text pointer */
	}
    }
    /********************************************************************
     *  Initialise c_number, temp C file T1FP and functionUse[1].c.expr
     *******************************************************************/
    iC_genLineNums[0] = 0;
    while ((ca = ArExpr[c_number][0]) != NULL) {
	functionUse[0].c_cnt |= F_ARITHM;	/* flag for copying macros */
	cp = lineBuf;
	vFlag = 0;
	while ((c = (*ca++) & 0377) != 0) {
	    if (vFlag) {
		cp += sprintf(cp, MV "(%d)", c);
		vFlag = 0;
	    } else if (c == 0377) {		/* variable marker ('\377' compiles to -1 !!) */
		vFlag = 1;
	    } else {
		*cp++ = c;			/* operators in the expression */
	    }
	    assert(cp < &lineBuf[BUFS]);	/* make sure pre-compiled expressions don't overflow */	
	}
	*cp++ = '\0';				/* terminate expression string */
	assert(vFlag == 0);			/* make sure each variable has a variable number */
	ca = ArExpr[c_number][0];		/* clone macro again */
	cp = ArExpr[c_number][1];		/* #line string */
	writeCexeString(T1FP, ++c_number);	/* and record for copying */
	fprintf(T1FP, "%s\n", cp);		/* #line string */
	fprintf(T1FP, "	return %s;\n", lineBuf);/* intial iC_x() text for SHR() and SHSR() */
	writeCexeTail(T1FP, "%##\n", c_number);
	if (sscanf(cp,
#ifdef	_WIN32
	    " # line %d \"%[-/:A-Za-z_.0-9<>]\"	%s",
#else	/* ! _WIN32 Linux */
	    " # line %d \"%[-/A-Za-z_.0-9<>]\"	%s",
#endif	/* _WIN32 */
	    &num, ids, lineBuf) < 2) {
	    assert(0);				/* error in generating code in outp.c */
	}
	assert(c_number < GEN_MAX);
	iC_genLineNums[c_number] = num;		/* line number from comment line */
	if (iC_genName) {
	    assert(strcmp(iC_genName, ids) == 0);/* file names in comment lines should all be the same */
	} else {
	    iC_genName = iC_emalloc(strlen(ids) + 1);/* +1 for '\0' */
	    strcpy(iC_genName, ids);
	}
	cp = iC_emalloc(strlen(ca) + 1);	/* +1 for '\0' */
	strcpy(cp, ca);				/* must use emalloc() because cp if free'd in outp.c */
	assert(c_number < functionUseSize);
	functionUse[c_number].c.expr = cp;	/* C expression prepared for cloning */
	iC_genCount++;
    }
#endif	/* BOOT_COMPILE */

    /********************************************************************
     * Build temporary files for linking multiple sources
     *******************************************************************/
    if (iC_Lflag != 0) {
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
} /* iC_init */

static const char init_c[] =
"@(#)$Id: init.c 1.46 $";
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

static Symbol	SR_a_2     = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[24] };
static Symbol	SR_a_1     = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[22], .next=&SR_a_2 };

static Symbol	DR_a_2     = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[37] };
static Symbol	DR_a_1     = { .type=OR , .ftype=D_FF  , .fm=0x41, .u_blist=&l[35], .next=&DR_a_2 };

static Symbol	SRR_a_3    = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[53] };
static Symbol	SRR_a_2    = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[51], .next=&SRR_a_3 };
static Symbol	SRR_a_1    = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[49], .next=&SRR_a_2 };

static Symbol	DSR_a_3    = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[71] };
static Symbol	DSR_a_2    = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[69], .next=&DSR_a_3 };
static Symbol	DSR_a_1    = { .type=OR , .ftype=D_FF  , .fm=0x41, .u_blist=&l[67], .next=&DSR_a_2 };

static Symbol	SHa_1      = { .type=ARN, .ftype=D_SH  , .fm=0x41, .u_blist=&l[83] };

static Symbol	SHR_a_2    = { .type=OR , .ftype=R_SH  , .fm=0x41, .u_blist=&l[94] };
static Symbol	SHR_a_1    = { .type=ARN, .ftype=D_SH  , .fm=0x41, .u_blist=&l[92], .next=&SHR_a_2 };

static Symbol	SHSR_a_3   = { .type=OR , .ftype=R_SH  , .fm=0x41, .u_blist=&l[110] };
static Symbol	SHSR_a_2   = { .type=OR , .ftype=S_SH  , .fm=0x41, .u_blist=&l[108], .next=&SHSR_a_3 };
static Symbol	SHSR_a_1   = { .type=ARN, .ftype=D_SH  , .fm=0x41, .u_blist=&l[106], .next=&SHSR_a_2 };

static Symbol	RISEa_1    = { .type=OR , .ftype=RI_BIT, .fm=0x41, .u_blist=&l[122] };

static Symbol	CHANGEa_1  = { .type=OR , .ftype=CH_BIT, .fm=0x41, .u_blist=&l[130] };

static Symbol	CHANGE2a_1 = { .type=ARN, .ftype=CH_AR , .fm=0x41, .u_blist=&l[138] };

static Symbol	CLOCKa_1   = { .type=OR , .ftype=CLCK  , .fm=0x41, .u_blist=&l[146] };

static Symbol	CLOCK2a_2  = { .type=OR , .ftype=CLCK  , .fm=0x41, .u_blist=&l[157] };
static Symbol	CLOCK2a_1  = { .type=OR , .ftype=CLCK  , .fm=0x41, .u_blist=&l[155], .next=&CLOCK2a_2 };

static Symbol	TIMERa_1   = { .type=OR , .ftype=TIMR  , .fm=0x41, .u_blist=&l[167] };

static Symbol	TIMER02a_2 = { .type=OR , .ftype=TIMR  , .fm=0x41, .u_blist=&l[178] };
static Symbol	TIMER02a_1 = { .type=OR , .ftype=TIMR  , .fm=0x41, .u_blist=&l[176], .next=&TIMER02a_2 };

static Symbol	TIMER1a_1  = { .type=OR , .ftype=TIMR  , .fm=0x41, .u_blist=&l[188] };

static Symbol	TIMER12a_2 = { .type=OR , .ftype=TIMR  , .fm=0x41, .u_blist=&l[199] };
static Symbol	TIMER12a_1 = { .type=OR , .ftype=TIMR  , .fm=0x41, .u_blist=&l[197], .next=&TIMER12a_2 };

static Symbol	SRa_2      = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[212] };
static Symbol	SRa_1      = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[210], .next=&SRa_2 };

static Symbol	SRXa_2     = { .type=AND, .ftype=R_FF  , .fm=0x41, .u_blist=&l[226] };
static Symbol	SRXa_1     = { .type=AND, .ftype=S_FF  , .fm=0x41, .u_blist=&l[223], .next=&SRXa_2 };

static Symbol	JKa_2      = { .type=AND, .ftype=R_FF  , .fm=0x41, .u_blist=&l[241] };
static Symbol	JKa_1      = { .type=AND, .ftype=S_FF  , .fm=0x41, .u_blist=&l[238], .next=&JKa_2 };

static Symbol	SRRa_3     = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[258] };
static Symbol	SRRa_2     = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[256], .next=&SRRa_3 };
static Symbol	SRRa_1     = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[254], .next=&SRRa_2 };

static Symbol	DRa_2      = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[278] };
static Symbol	DRa_1      = { .type=OR , .ftype=D_FF  , .fm=0x41, .u_blist=&l[275], .next=&DRa_2 };
static Symbol	DRa_4      = { .type=AND, .ftype=GATE  , .fm=0x41, .u_blist=&l[273], .next=&DRa_1 };
static Symbol	DRa_3      = { .type=AND, .ftype=GATE  , .fm=0x41, .u_blist=&l[271], .next=&DRa_4 };

static Symbol	DSRa_3     = { .type=AND, .ftype=R_FF  , .fm=0x41, .u_blist=&l[304] };
static Symbol	DSRa_2     = { .type=AND, .ftype=S_FF  , .fm=0x41, .u_blist=&l[301], .next=&DSRa_3 };
static Symbol	DSRa_1     = { .type=OR , .ftype=D_FF  , .fm=0x41, .u_blist=&l[298], .next=&DSRa_2 };
static Symbol	DSRa_5     = { .type=AND, .ftype=GATE  , .fm=0x41, .u_blist=&l[296], .next=&DSRa_1 };
static Symbol	DSRa_4     = { .type=AND, .ftype=GATE  , .fm=0x41, .u_blist=&l[294], .next=&DSRa_5 };

static Symbol	SHRa_2     = { .type=OR , .ftype=R_SH  , .fm=0x41, .u_blist=&l[323] };
static Symbol	SHRa_1     = { .type=ARN, .ftype=D_SH  , .fm=0x41, .u_blist=&l[319], .next=&SHRa_2 };
static Symbol	SHRa_3     = { .type=OR , .ftype=ARITH , .fm=0x41, .u_blist=&l[318], .next=&SHRa_1 };

static Symbol	SHSRa_3    = { .type=AND, .ftype=R_SH  , .fm=0x41, .u_blist=&l[344] };
static Symbol	SHSRa_2    = { .type=AND, .ftype=S_SH  , .fm=0x41, .u_blist=&l[341], .next=&SHSRa_3 };
static Symbol	SHSRa_1    = { .type=ARN, .ftype=D_SH  , .fm=0x41, .u_blist=&l[337], .next=&SHSRa_2 };
static Symbol	SHSRa_4    = { .type=OR , .ftype=ARITH , .fm=0x41, .u_blist=&l[335], .next=&SHSRa_1 };

static Symbol	STa_2      = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[360] };
static Symbol	STa_1      = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[358], .next=&STa_2 };

static Symbol	SRTa_3     = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[375] };
static Symbol	SRTa_2     = { .type=OR , .ftype=R_FF  , .fm=0x41, .u_blist=&l[373], .next=&SRTa_3 };
static Symbol	SRTa_1     = { .type=OR , .ftype=S_FF  , .fm=0x41, .u_blist=&l[371], .next=&SRTa_2 };

static Symbol	FALLa_1    = { .type=OR , .ftype=RI_BIT, .fm=0x41, .u_blist=&l[386] };


static Symbol	DLATCHa_1  = { .type=LATCH, .ftype=D_FF  , .fm=0x41, .u_blist=&l[402] };


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

/*  17 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR_ */
/*  18 */ { 0          , 0        , &l[19] , 0             , 0 },	/* ==>  SR_@ */
/*  19 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/*  20 */ { &SR_a_1    , 0        , &l[21] , (char*)&l[19] , 0 },	/* ==>  SR_a_1 */
/*  21 */ { &SR_a_2    , 0        , 0      , 0             , 0 },	/* ==>  SR_a_2 */
/*  22 */ { 0          , 0        , &l[23] , 0             , 0 },	/* ==>  SR_@scl */
/*  23 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR_@set */
/*  24 */ { 0          , 0        , &l[25] , 0             , 0 },	/* ==>  SR_@rcl */
/*  25 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR_@res */
/*  26 */ { 0          , 0        , &l[27] , 0             , 0 },	/* ==>  SR_@set */
/*  27 */ { 0          , 0        , &l[28] , 0             , 0 },	/* ==>  SR_@scl */
/*  28 */ { 0          , 0        , &l[29] , 0             , 0 },	/* ==>  SR_@res */
/*  29 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR_@rcl */

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

/*  43 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR_ */
/*  44 */ { 0          , 0        , &l[45] , 0             , 0 },	/* ==>  SRR_@ */
/*  45 */ { 0          , 6        , 0      , 0             , 0 },	/* ==>  var list */
/*  46 */ { &SRR_a_1   , 0        , &l[47] , (char*)&l[45] , 0 },	/* ==>  SRR_a_1 */
/*  47 */ { &SRR_a_2   , 0        , &l[48] , 0             , 0 },	/* ==>  SRR_a_2 */
/*  48 */ { &SRR_a_3   , 0        , 0      , 0             , 0 },	/* ==>  SRR_a_3 */
/*  49 */ { 0          , 0        , &l[50] , 0             , 0 },	/* ==>  SRR_@scl */
/*  50 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR_@set */
/*  51 */ { 0          , 0        , &l[52] , 0             , 0 },	/* ==>  SRR_@rcl */
/*  52 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR_@res */
/*  53 */ { 0          , 0        , &l[54] , 0             , 0 },	/* ==>  SRR_@r2cl */
/*  54 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR_@res2 */
/*  55 */ { 0          , 0        , &l[56] , 0             , 0 },	/* ==>  SRR_@set */
/*  56 */ { 0          , 0        , &l[57] , 0             , 0 },	/* ==>  SRR_@scl */
/*  57 */ { 0          , 0        , &l[58] , 0             , 0 },	/* ==>  SRR_@res */
/*  58 */ { 0          , 0        , &l[59] , 0             , 0 },	/* ==>  SRR_@rcl */
/*  59 */ { 0          , 0        , &l[60] , 0             , 0 },	/* ==>  SRR_@res2 */
/*  60 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR_@r2cl */

/*  61 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR_ */
/*  62 */ { 0          , 0        , &l[63] , 0             , 0 },	/* ==>  DSR_@ */
/*  63 */ { 0          , 6        , 0      , 0             , 0 },	/* ==>  var list */
/*  64 */ { &DSR_a_1   , 0        , &l[65] , (char*)&l[63] , 0 },	/* ==>  DSR_a_1 */
/*  65 */ { &DSR_a_2   , 0        , &l[66] , 0             , 0 },	/* ==>  DSR_a_2 */
/*  66 */ { &DSR_a_3   , 0        , 0      , 0             , 0 },	/* ==>  DSR_a_3 */
/*  67 */ { 0          , 0        , &l[68] , 0             , 0 },	/* ==>  DSR_@dcl */
/*  68 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR_@expr */
/*  69 */ { 0          , 0        , &l[70] , 0             , 0 },	/* ==>  DSR_@scl */
/*  70 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR_@set */
/*  71 */ { 0          , 0        , &l[72] , 0             , 0 },	/* ==>  DSR_@rcl */
/*  72 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR_@res */
/*  73 */ { 0          , 0        , &l[74] , 0             , 0 },	/* ==>  DSR_@expr */
/*  74 */ { 0          , 0        , &l[75] , 0             , 0 },	/* ==>  DSR_@dcl */
/*  75 */ { 0          , 0        , &l[76] , 0             , 0 },	/* ==>  DSR_@set */
/*  76 */ { 0          , 0        , &l[77] , 0             , 0 },	/* ==>  DSR_@scl */
/*  77 */ { 0          , 0        , &l[78] , 0             , 0 },	/* ==>  DSR_@res */
/*  78 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR_@rcl */

/*  79 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SH */
/*  80 */ { 0          , 0        , &l[81] , 0             , 0 },	/* ==>  SH@ */
/*  81 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/*  82 */ { &SHa_1     , 0        , 0      , (char*)&l[81] , 0 },	/* ==>  SHa_1 */
/*  83 */ { 0          , 0        , &l[84] , 0             , 0 },	/* ==>  SH@dcl */
/*  84 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SH@dat */
/*  85 */ { 0          , 0        , &l[86] , 0             , 0 },	/* ==>  SH@dat */
/*  86 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SH@dcl */

/*  87 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR_ */
/*  88 */ { 0          , 0        , &l[89] , 0             , 0 },	/* ==>  SHR_@ */
/*  89 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/*  90 */ { &SHR_a_1   , 0        , &l[91] , (char*)&l[89] , 0 },	/* ==>  SHR_a_1 */
/*  91 */ { &SHR_a_2   , 0        , 0      , 0             , 0 },	/* ==>  SHR_a_2 */
/*  92 */ { 0          , 0        , &l[93] , 0             , 0 },	/* ==>  SHR_@dcl */
/*  93 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR_@dat */
/*  94 */ { 0          , 0        , &l[95] , 0             , 0 },	/* ==>  SHR_@rcl */
/*  95 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR_@res */
/*  96 */ { 0          , 0        , &l[97] , 0             , 0 },	/* ==>  SHR_@dat */
/*  97 */ { 0          , 0        , &l[98] , 0             , 0 },	/* ==>  SHR_@dcl */
/*  98 */ { 0          , 0        , &l[99] , 0             , 0 },	/* ==>  SHR_@res */
/*  99 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR_@rcl */

/* 100 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR_ */
/* 101 */ { 0          , 0        , &l[102], 0             , 0 },	/* ==>  SHSR_@ */
/* 102 */ { 0          , 6        , 0      , 0             , 0 },	/* ==>  var list */
/* 103 */ { &SHSR_a_1  , 0        , &l[104], (char*)&l[102], 0 },	/* ==>  SHSR_a_1 */
/* 104 */ { &SHSR_a_2  , 0        , &l[105], 0             , 0 },	/* ==>  SHSR_a_2 */
/* 105 */ { &SHSR_a_3  , 0        , 0      , 0             , 0 },	/* ==>  SHSR_a_3 */
/* 106 */ { 0          , 0        , &l[107], 0             , 0 },	/* ==>  SHSR_@dcl */
/* 107 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR_@dat */
/* 108 */ { 0          , 0        , &l[109], 0             , 0 },	/* ==>  SHSR_@scl */
/* 109 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR_@set */
/* 110 */ { 0          , 0        , &l[111], 0             , 0 },	/* ==>  SHSR_@rcl */
/* 111 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR_@res */
/* 112 */ { 0          , 0        , &l[113], 0             , 0 },	/* ==>  SHSR_@dat */
/* 113 */ { 0          , 0        , &l[114], 0             , 0 },	/* ==>  SHSR_@dcl */
/* 114 */ { 0          , 0        , &l[115], 0             , 0 },	/* ==>  SHSR_@set */
/* 115 */ { 0          , 0        , &l[116], 0             , 0 },	/* ==>  SHSR_@scl */
/* 116 */ { 0          , 0        , &l[117], 0             , 0 },	/* ==>  SHSR_@res */
/* 117 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR_@rcl */

/* 118 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  RISE */
/* 119 */ { 0          , 0        , &l[120], 0             , 0 },	/* ==>  RISE@ */
/* 120 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 121 */ { &RISEa_1   , 0        , 0      , (char*)&l[120], 0 },	/* ==>  RISEa_1 */
/* 122 */ { 0          , 0        , &l[123], 0             , 0 },	/* ==>  RISE@clk */
/* 123 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  RISE@trig */
/* 124 */ { 0          , 0        , &l[125], 0             , 0 },	/* ==>  RISE@trig */
/* 125 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  RISE@clk */

/* 126 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CHANGE */
/* 127 */ { 0          , 0        , &l[128], 0             , 0 },	/* ==>  CHANGE@ */
/* 128 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 129 */ { &CHANGEa_1 , 0        , 0      , (char*)&l[128], 0 },	/* ==>  CHANGEa_1 */
/* 130 */ { 0          , 0        , &l[131], 0             , 0 },	/* ==>  CHANGE@clk */
/* 131 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CHANGE@trig */
/* 132 */ { 0          , 0        , &l[133], 0             , 0 },	/* ==>  CHANGE@trig */
/* 133 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CHANGE@clk */

/* 134 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CHANGE2 */
/* 135 */ { 0          , 0        , &l[136], 0             , 0 },	/* ==>  CHANGE2@ */
/* 136 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 137 */ { &CHANGE2a_1, 0        , 0      , (char*)&l[136], 0 },	/* ==>  CHANGE2a_1 */
/* 138 */ { 0          , 0        , &l[139], 0             , 0 },	/* ==>  CHANGE2@clk */
/* 139 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CHANGE2@val */
/* 140 */ { 0          , 0        , &l[141], 0             , 0 },	/* ==>  CHANGE2@val */
/* 141 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CHANGE2@clk */

/* 142 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK */
/* 143 */ { 0          , 0        , &l[144], 0             , 0 },	/* ==>  CLOCK@ */
/* 144 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 145 */ { &CLOCKa_1  , 0        , 0      , (char*)&l[144], 0 },	/* ==>  CLOCKa_1 */
/* 146 */ { 0          , 0        , &l[147], 0             , 0 },	/* ==>  CLOCK@clk */
/* 147 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK@trig */
/* 148 */ { 0          , 0        , &l[149], 0             , 0 },	/* ==>  CLOCK@trig */
/* 149 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK@clk */

/* 150 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK2 */
/* 151 */ { 0          , 0        , &l[152], 0             , 0 },	/* ==>  CLOCK2@ */
/* 152 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 153 */ { &CLOCK2a_1 , 0        , &l[154], (char*)&l[152], 0 },	/* ==>  CLOCK2a_1 */
/* 154 */ { &CLOCK2a_2 , 0        , 0      , 0             , 0 },	/* ==>  CLOCK2a_2 */
/* 155 */ { 0          , 0        , &l[156], 0             , 0 },	/* ==>  CLOCK2@clk */
/* 156 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK2@trig */
/* 157 */ { 0          , 0        , &l[158], 0             , 0 },	/* ==>  CLOCK2@clk2 */
/* 158 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK2@trig2 */
/* 159 */ { 0          , 0        , &l[160], 0             , 0 },	/* ==>  CLOCK2@trig */
/* 160 */ { 0          , 0        , &l[161], 0             , 0 },	/* ==>  CLOCK2@clk */
/* 161 */ { 0          , 0        , &l[162], 0             , 0 },	/* ==>  CLOCK2@trig2 */
/* 162 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  CLOCK2@clk2 */

/* 163 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER */
/* 164 */ { 0          , 0        , &l[165], 0             , 0 },	/* ==>  TIMER@ */
/* 165 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 166 */ { &TIMERa_1  , 0        , 0      , (char*)&l[165], 0 },	/* ==>  TIMERa_1 */
/* 167 */ { 0          , 0        , &l[168], 0             , 0 },	/* ==>  TIMER@clk */
/* 168 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER@trig */
/* 169 */ { 0          , 0        , &l[170], 0             , 0 },	/* ==>  TIMER@trig */
/* 170 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER@clk */

/* 171 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER02 */
/* 172 */ { 0          , 0        , &l[173], 0             , 0 },	/* ==>  TIMER02@ */
/* 173 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 174 */ { &TIMER02a_1, 0        , &l[175], (char*)&l[173], 0 },	/* ==>  TIMER02a_1 */
/* 175 */ { &TIMER02a_2, 0        , 0      , 0             , 0 },	/* ==>  TIMER02a_2 */
/* 176 */ { 0          , 0        , &l[177], 0             , 0 },	/* ==>  TIMER02@clk */
/* 177 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER02@trig */
/* 178 */ { 0          , 0        , &l[179], 0             , 0 },	/* ==>  TIMER02@clk2 */
/* 179 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER02@trig2 */
/* 180 */ { 0          , 0        , &l[181], 0             , 0 },	/* ==>  TIMER02@trig */
/* 181 */ { 0          , 0        , &l[182], 0             , 0 },	/* ==>  TIMER02@clk */
/* 182 */ { 0          , 0        , &l[183], 0             , 0 },	/* ==>  TIMER02@trig2 */
/* 183 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER02@clk2 */

/* 184 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER1 */
/* 185 */ { 0          , 0        , &l[186], 0             , 0 },	/* ==>  TIMER1@ */
/* 186 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 187 */ { &TIMER1a_1 , 1        , 0      , (char*)&l[186], 0 },	/* ==> ~TIMER1a_1 */
/* 188 */ { 0          , 0        , &l[189], 0             , 0 },	/* ==>  TIMER1@clk */
/* 189 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER1@trig */
/* 190 */ { 0          , 0        , &l[191], 0             , 0 },	/* ==>  TIMER1@trig */
/* 191 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER1@clk */

/* 192 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER12 */
/* 193 */ { 0          , 0        , &l[194], 0             , 0 },	/* ==>  TIMER12@ */
/* 194 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 195 */ { &TIMER12a_1, 1        , &l[196], (char*)&l[194], 0 },	/* ==> ~TIMER12a_1 */
/* 196 */ { &TIMER12a_2, 0        , 0      , 0             , 0 },	/* ==>  TIMER12a_2 */
/* 197 */ { 0          , 0        , &l[198], 0             , 0 },	/* ==>  TIMER12@clk */
/* 198 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER12@trig */
/* 199 */ { 0          , 0        , &l[200], 0             , 0 },	/* ==>  TIMER12@clk2 */
/* 200 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER12@trig2 */
/* 201 */ { 0          , 0        , &l[202], 0             , 0 },	/* ==>  TIMER12@trig */
/* 202 */ { 0          , 0        , &l[203], 0             , 0 },	/* ==>  TIMER12@clk */
/* 203 */ { 0          , 0        , &l[204], 0             , 0 },	/* ==>  TIMER12@trig2 */
/* 204 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  TIMER12@clk2 */

/* 205 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR */
/* 206 */ { 0          , 0        , &l[207], 0             , 0 },	/* ==>  SR@ */
/* 207 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 208 */ { &SRa_1     , 0        , &l[209], (char*)&l[207], 0 },	/* ==>  SRa_1 */
/* 209 */ { &SRa_2     , 0        , 0      , 0             , 0 },	/* ==>  SRa_2 */
/* 210 */ { 0          , 0        , &l[211], 0             , 0 },	/* ==>  SR@scl */
/* 211 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR@set */
/* 212 */ { 0          , 0        , &l[213], 0             , 0 },	/* ==>  SR@rcl */
/* 213 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR@res */
/* 214 */ { 0          , 0        , &l[215], 0             , 0 },	/* ==>  SR@set */
/* 215 */ { 0          , 0        , &l[216], 0             , 0 },	/* ==>  SR@scl */
/* 216 */ { 0          , 0        , &l[217], 0             , 0 },	/* ==>  SR@res */
/* 217 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SR@rcl */

/* 218 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRX */
/* 219 */ { 0          , 0        , &l[220], 0             , 0 },	/* ==>  SRX@ */
/* 220 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 221 */ { &SRXa_1    , 0        , &l[222], (char*)&l[220], 0 },	/* ==>  SRXa_1 */
/* 222 */ { &SRXa_2    , 0        , 0      , 0             , 0 },	/* ==>  SRXa_2 */
/* 223 */ { 0          , 0        , &l[224], 0             , 0 },	/* ==>  SRX@scl */
/* 224 */ { 0          , 0        , &l[225], 0             , 0 },	/* ==>  SRX@set */
/* 225 */ { 0          , 1        , 0      , 0             , 0 },	/* ==> ~SRX@res */
/* 226 */ { 0          , 0        , &l[227], 0             , 0 },	/* ==>  SRX@rcl */
/* 227 */ { 0          , 1        , &l[228], (char*)&l[224], 0 },	/* ==> ~SRX@set */
/* 228 */ { 0          , 0        , 0      , (char*)&l[225], 0 },	/* ==>  SRX@res */
/* 229 */ { 0          , 0        , &l[230], 0             , 0 },	/* ==>  SRX@set */
/* 230 */ { 0          , 0        , &l[231], 0             , 0 },	/* ==>  SRX@scl */
/* 231 */ { 0          , 0        , &l[232], 0             , 0 },	/* ==>  SRX@res */
/* 232 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRX@rcl */

/* 233 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  JK */
/* 234 */ { 0          , 0        , &l[235], 0             , 0 },	/* ==>  JK@ */
/* 235 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 236 */ { &JKa_1     , 0        , &l[237], (char*)&l[235], 0 },	/* ==>  JKa_1 */
/* 237 */ { &JKa_2     , 0        , 0      , 0             , 0 },	/* ==>  JKa_2 */
/* 238 */ { 0          , 0        , &l[239], 0             , 0 },	/* ==>  JK@scl */
/* 239 */ { 0          , 1        , &l[240], 0             , 0 },	/* ==> ~JK@ */
/* 240 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  JK@set */
/* 241 */ { 0          , 0        , &l[242], 0             , 0 },	/* ==>  JK@rcl */
/* 242 */ { 0          , 0        , &l[243], (char*)&l[239], 0 },	/* ==>  JK@ */
/* 243 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  JK@res */
/* 244 */ { 0          , 0        , &l[245], 0             , 0 },	/* ==>  JK@set */
/* 245 */ { 0          , 0        , &l[246], 0             , 0 },	/* ==>  JK@scl */
/* 246 */ { 0          , 0        , &l[247], 0             , 0 },	/* ==>  JK@res */
/* 247 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  JK@rcl */

/* 248 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR */
/* 249 */ { 0          , 0        , &l[250], 0             , 0 },	/* ==>  SRR@ */
/* 250 */ { 0          , 6        , 0      , 0             , 0 },	/* ==>  var list */
/* 251 */ { &SRRa_1    , 0        , &l[252], (char*)&l[250], 0 },	/* ==>  SRRa_1 */
/* 252 */ { &SRRa_2    , 0        , &l[253], 0             , 0 },	/* ==>  SRRa_2 */
/* 253 */ { &SRRa_3    , 0        , 0      , 0             , 0 },	/* ==>  SRRa_3 */
/* 254 */ { 0          , 0        , &l[255], 0             , 0 },	/* ==>  SRR@scl */
/* 255 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR@set */
/* 256 */ { 0          , 0        , &l[257], 0             , 0 },	/* ==>  SRR@rcl */
/* 257 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR@res */
/* 258 */ { 0          , 0        , &l[259], 0             , 0 },	/* ==>  SRR@r2cl */
/* 259 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR@res2 */
/* 260 */ { 0          , 0        , &l[261], 0             , 0 },	/* ==>  SRR@set */
/* 261 */ { 0          , 0        , &l[262], 0             , 0 },	/* ==>  SRR@scl */
/* 262 */ { 0          , 0        , &l[263], 0             , 0 },	/* ==>  SRR@res */
/* 263 */ { 0          , 0        , &l[264], 0             , 0 },	/* ==>  SRR@rcl */
/* 264 */ { 0          , 0        , &l[265], 0             , 0 },	/* ==>  SRR@res2 */
/* 265 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRR@r2cl */

/* 266 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DR */
/* 267 */ { 0          , 0        , &l[268], 0             , 0 },	/* ==>  DR@ */
/* 268 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 269 */ { &DRa_1     , 0        , &l[270], 0             , 0 },	/* ==>  DRa_1 */
/* 270 */ { &DRa_2     , 0        , 0      , 0             , 0 },	/* ==>  DRa_2 */
/* 271 */ { 0          , 0        , &l[272], 0             , 0 },	/* ==>  DR@dat */
/* 272 */ { 0          , 1        , 0      , 0             , 0 },	/* ==> ~DR@res */
/* 273 */ { 0          , 0        , &l[274], 0             , 0 },	/* ==>  DR@ */
/* 274 */ { 0          , 0        , 0      , (char*)&l[272], 0 },	/* ==>  DR@res */
/* 275 */ { 0          , 0        , &l[276], 0             , 0 },	/* ==>  DR@dcl */
/* 276 */ { &DRa_3     , 0        , &l[277], (char*)&l[268], 0 },	/* ==>  DRa_3 */
/* 277 */ { &DRa_4     , 0        , 0      , 0             , 0 },	/* ==>  DRa_4 */
/* 278 */ { 0          , 0        , &l[279], 0             , 0 },	/* ==>  DR@rcl */
/* 279 */ { 0          , 0        , 0      , (char*)&l[274], 0 },	/* ==>  DR@res */
/* 280 */ { 0          , 0        , &l[281], 0             , 0 },	/* ==>  DR@dat */
/* 281 */ { 0          , 0        , &l[282], 0             , 0 },	/* ==>  DR@dcl */
/* 282 */ { 0          , 0        , &l[283], 0             , 0 },	/* ==>  DR@res */
/* 283 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DR@rcl */

/* 284 */ { 0          , 0        , &l[285], 0             , 0 },	/* ==>  DSR@i */
/* 285 */ { 0          , 7        , &l[287], 0             , 0 },	/* ==>  no var list */
/* 286 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR */
/* 287 */ { 0          , 0        , &l[288], 0             , 0 },	/* ==>  DSR@ */
/* 288 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  var list */
/* 289 */ { 0          , 0        , &l[290], 0             , 0 },	/* ==>  DSR@set */
/* 290 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR@res */
/* 291 */ { &DSRa_1    , 0        , &l[292], 0             , 0 },	/* ==>  DSRa_1 */
/* 292 */ { &DSRa_2    , 0        , &l[293], 0             , 0 },	/* ==>  DSRa_2 */
/* 293 */ { &DSRa_3    , 0        , 0      , 0             , 0 },	/* ==>  DSRa_3 */
/* 294 */ { 0          , 0        , &l[295], 0             , 0 },	/* ==>  DSR@dat */
/* 295 */ { 0          , 1        , 0      , (char*)&l[284], 0 },	/* ==> ~DSR@i */
/* 296 */ { 0          , 0        , &l[297], 0             , 0 },	/* ==>  DSR@ */
/* 297 */ { 0          , 0        , 0      , (char*)&l[295], 0 },	/* ==>  DSR@i */
/* 298 */ { 0          , 0        , &l[299], 0             , 0 },	/* ==>  DSR@dcl */
/* 299 */ { &DSRa_4    , 0        , &l[300], (char*)&l[288], 0 },	/* ==>  DSRa_4 */
/* 300 */ { &DSRa_5    , 0        , 0      , 0             , 0 },	/* ==>  DSRa_5 */
/* 301 */ { 0          , 0        , &l[302], 0             , 0 },	/* ==>  DSR@scl */
/* 302 */ { 0          , 0        , &l[303], (char*)&l[289], 0 },	/* ==>  DSR@set */
/* 303 */ { 0          , 1        , 0      , (char*)&l[290], 0 },	/* ==> ~DSR@res */
/* 304 */ { 0          , 0        , &l[305], 0             , 0 },	/* ==>  DSR@rcl */
/* 305 */ { 0          , 1        , &l[306], (char*)&l[302], 0 },	/* ==> ~DSR@set */
/* 306 */ { 0          , 0        , 0      , (char*)&l[303], 0 },	/* ==>  DSR@res */
/* 307 */ { 0          , 0        , &l[308], 0             , 0 },	/* ==>  DSR@dat */
/* 308 */ { 0          , 0        , &l[309], 0             , 0 },	/* ==>  DSR@dcl */
/* 309 */ { 0          , 0        , &l[310], 0             , 0 },	/* ==>  DSR@set */
/* 310 */ { 0          , 0        , &l[311], 0             , 0 },	/* ==>  DSR@scl */
/* 311 */ { 0          , 0        , &l[312], 0             , 0 },	/* ==>  DSR@res */
/* 312 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DSR@rcl */

/* 313 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR */
/* 314 */ { 0          , 0        , &l[315], 0             , 0 },	/* ==>  SHR@ */
/* 315 */ { 0          , 4        , 0      , 0             , 0 },	/* ==>  var list */
/* 316 */ { &SHRa_1    , 0        , &l[317], 0             , 0 },	/* ==>  SHRa_1 */
/* 317 */ { &SHRa_2    , 0        , 0      , 0             , 0 },	/* ==>  SHRa_2 */
/* 318 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR@res */
/* 319 */ { 0          , 0        , &l[320], 0             , 0 },	/* ==>  SHR@dcl */
/* 320 */ { &SHRa_3    , (1<<FO)+1, &l[321], (char*)&l[315], 0 },	/* ==>  SHRa_3 */
/* 321 */ { 0          , (1<<FO)+2, &l[322], 0             , 0 },	/* ==>  SHR@ */
/* 322 */ { 0          , (1<<FO)+3, 0      , 0             , 0 },	/* ==>  SHR@dat */
/* 323 */ { 0          , 0        , &l[324], 0             , 0 },	/* ==>  SHR@rcl */
/* 324 */ { 0          , 0        , 0      , (char*)&l[318], 0 },	/* ==>  SHR@res */
/* 325 */ { 0          , 0        , &l[326], 0             , 0 },	/* ==>  SHR@dat */
/* 326 */ { 0          , 0        , &l[327], 0             , 0 },	/* ==>  SHR@dcl */
/* 327 */ { 0          , 0        , &l[328], 0             , 0 },	/* ==>  SHR@res */
/* 328 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHR@rcl */

/* 329 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR */
/* 330 */ { 0          , 0        , &l[331], 0             , 0 },	/* ==>  SHSR@ */
/* 331 */ { 0          , 6        , 0      , 0             , 0 },	/* ==>  var list */
/* 332 */ { &SHSRa_1   , 0        , &l[333], 0             , 0 },	/* ==>  SHSRa_1 */
/* 333 */ { &SHSRa_2   , 0        , &l[334], 0             , 0 },	/* ==>  SHSRa_2 */
/* 334 */ { &SHSRa_3   , 0        , 0      , 0             , 0 },	/* ==>  SHSRa_3 */
/* 335 */ { 0          , 0        , &l[336], 0             , 0 },	/* ==>  SHSR@set */
/* 336 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR@res */
/* 337 */ { 0          , 0        , &l[338], 0             , 0 },	/* ==>  SHSR@dcl */
/* 338 */ { &SHSRa_4   , (1<<FO)+1, &l[339], (char*)&l[331], 0 },	/* ==>  SHSRa_4 */
/* 339 */ { 0          , (1<<FO)+2, &l[340], 0             , 0 },	/* ==>  SHSR@ */
/* 340 */ { 0          , (1<<FO)+3, 0      , 0             , 0 },	/* ==>  SHSR@dat */
/* 341 */ { 0          , 0        , &l[342], 0             , 0 },	/* ==>  SHSR@scl */
/* 342 */ { 0          , 0        , &l[343], (char*)&l[335], 0 },	/* ==>  SHSR@set */
/* 343 */ { 0          , 1        , 0      , (char*)&l[336], 0 },	/* ==> ~SHSR@res */
/* 344 */ { 0          , 0        , &l[345], 0             , 0 },	/* ==>  SHSR@rcl */
/* 345 */ { 0          , 1        , &l[346], (char*)&l[342], 0 },	/* ==> ~SHSR@set */
/* 346 */ { 0          , 0        , 0      , (char*)&l[343], 0 },	/* ==>  SHSR@res */
/* 347 */ { 0          , 0        , &l[348], 0             , 0 },	/* ==>  SHSR@dat */
/* 348 */ { 0          , 0        , &l[349], 0             , 0 },	/* ==>  SHSR@dcl */
/* 349 */ { 0          , 0        , &l[350], 0             , 0 },	/* ==>  SHSR@set */
/* 350 */ { 0          , 0        , &l[351], 0             , 0 },	/* ==>  SHSR@scl */
/* 351 */ { 0          , 0        , &l[352], 0             , 0 },	/* ==>  SHSR@res */
/* 352 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SHSR@rcl */

/* 353 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  ST */
/* 354 */ { 0          , 0        , &l[355], 0             , 0 },	/* ==>  ST@ */
/* 355 */ { 0          , 3        , 0      , 0             , 0 },	/* ==>  var list */
/* 356 */ { &STa_1     , 0        , &l[357], (char*)&l[355], 0 },	/* ==>  STa_1 */
/* 357 */ { &STa_2     , 0        , 0      , 0             , 0 },	/* ==>  STa_2 */
/* 358 */ { 0          , 0        , &l[359], 0             , 0 },	/* ==>  ST@scl */
/* 359 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  ST@set */
/* 360 */ { 0          , 0        , &l[361], 0             , 0 },	/* ==>  ST@tcl */
/* 361 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  ST@ */
/* 362 */ { 0          , 0        , &l[363], 0             , 0 },	/* ==>  ST@set */
/* 363 */ { 0          , 0        , &l[364], 0             , 0 },	/* ==>  ST@scl */
/* 364 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  ST@tcl */

/* 365 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRT */
/* 366 */ { 0          , 0        , &l[367], 0             , 0 },	/* ==>  SRT@ */
/* 367 */ { 0          , 5        , 0      , 0             , 0 },	/* ==>  var list */
/* 368 */ { &SRTa_1    , 0        , &l[369], (char*)&l[367], 0 },	/* ==>  SRTa_1 */
/* 369 */ { &SRTa_2    , 0        , &l[370], 0             , 0 },	/* ==>  SRTa_2 */
/* 370 */ { &SRTa_3    , 0        , 0      , 0             , 0 },	/* ==>  SRTa_3 */
/* 371 */ { 0          , 0        , &l[372], 0             , 0 },	/* ==>  SRT@scl */
/* 372 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRT@set */
/* 373 */ { 0          , 0        , &l[374], 0             , 0 },	/* ==>  SRT@rcl */
/* 374 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRT@res */
/* 375 */ { 0          , 0        , &l[376], 0             , 0 },	/* ==>  SRT@tcl */
/* 376 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRT@ */
/* 377 */ { 0          , 0        , &l[378], 0             , 0 },	/* ==>  SRT@set */
/* 378 */ { 0          , 0        , &l[379], 0             , 0 },	/* ==>  SRT@scl */
/* 379 */ { 0          , 0        , &l[380], 0             , 0 },	/* ==>  SRT@res */
/* 380 */ { 0          , 0        , &l[381], 0             , 0 },	/* ==>  SRT@rcl */
/* 381 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  SRT@tcl */

/* 382 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  FALL */
/* 383 */ { 0          , 0        , &l[384], 0             , 0 },	/* ==>  FALL@ */
/* 384 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  var list */
/* 385 */ { &FALLa_1   , 0        , 0      , (char*)&l[384], 0 },	/* ==>  FALLa_1 */
/* 386 */ { 0          , 0        , &l[387], 0             , 0 },	/* ==>  FALL@clk */
/* 387 */ { 0          , 1        , 0      , 0             , 0 },	/* ==> ~FALL@fall */
/* 388 */ { 0          , 0        , &l[389], 0             , 0 },	/* ==>  FALL@fall */
/* 389 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  FALL@clk */

/* 390 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  LATCH */
/* 391 */ { 0          , 0        , &l[392], 0             , 0 },	/* ==>  LATCH@ */
/* 392 */ { 0          , 2        , 0      , 0             , 0 },	/* ==>  no var list */
/* 393 */ { 0          , 0        , &l[394], 0             , 0 },	/* ==>  LATCH@ */
/* 394 */ { 0          , 0        , &l[395], 0             , 0 },	/* ==>  LATCH@set */
/* 395 */ { 0          , 1        , 0      , 0             , 0 },	/* ==> ~LATCH@res */
/* 396 */ { 0          , 0        , &l[397], 0             , 0 },	/* ==>  LATCH@set */
/* 397 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  LATCH@res */

/* 398 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DLATCH */
/* 399 */ { 0          , 0        , &l[400], 0             , 0 },	/* ==>  DLATCH@ */
/* 400 */ { 0          , 3        , 0      , 0             , 0 },	/* ==>  var list */
/* 401 */ { &DLATCHa_1 , 0        , 0      , (char*)&l[400], 0 },	/* ==>  DLATCHa_1 */
/* 402 */ { 0          , 0        , &l[403], 0             , 0 },	/* ==>  DLATCH@clk */
/* 403 */ { 0          , 0        , &l[404], 0             , 0 },	/* ==>  DLATCH@ */
/* 404 */ { 0          , 0        , &l[405], 0             , 0 },	/* ==>  DLATCH@set */
/* 405 */ { 0          , 1        , 0      , 0             , 0 },	/* ==> ~DLATCH@res */
/* 406 */ { 0          , 0        , &l[407], 0             , 0 },	/* ==>  DLATCH@set */
/* 407 */ { 0          , 0        , &l[408], 0             , 0 },	/* ==>  DLATCH@res */
/* 408 */ { 0          , 0        , 0      , 0             , 0 },	/* ==>  DLATCH@clk */
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

  { "SR_",         IFUNCT,  GATE,  0x0f, &l[26] , &l[18] , &l[17] , 0               },	/* 9 pre-installed "SR_" function block */
  { "SR_@",          FF,    GATE,  0x8d, &l[17] , &l[20] , &l[18] , 0               },
  { "SR_@set",       OR,    GATE,  0x85, 0      , 0      , &l[23] , &l[26]          },
  { "SR_@scl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[22] , &l[27]          },
  { "SR_@res",       OR,    GATE,  0x85, 0      , 0      , &l[25] , &l[28]          },
  { "SR_@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[24] , &l[29]          },

  { "DR_",         IFUNCT,  GATE,  0x0f, &l[39] , &l[31] , &l[30] , 0               },	/* 15 pre-installed "DR_" function block */
  { "DR_@",          FF,    GATE,  0x8d, &l[30] , &l[33] , &l[31] , 0               },
  { "DR_@expr",      OR,    GATE,  0x85, 0      , 0      , &l[36] , &l[39]          },
  { "DR_@dcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[35] , &l[40]          },
  { "DR_@res",       OR,    GATE,  0x85, 0      , 0      , &l[38] , &l[41]          },
  { "DR_@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[37] , &l[42]          },

  { "SRR_",        IFUNCT,  GATE,  0x3f, &l[55] , &l[44] , &l[43] , 0               },	/* 21 pre-installed "SRR_" function block */
  { "SRR_@",         FF,    GATE,  0x8d, &l[43] , &l[46] , &l[44] , 0               },
  { "SRR_@set",      OR,    GATE,  0x85, 0      , 0      , &l[50] , &l[55]          },
  { "SRR_@scl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[49] , &l[56]          },
  { "SRR_@res",      OR,    GATE,  0x85, 0      , 0      , &l[52] , &l[57]          },
  { "SRR_@rcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[51] , &l[58]          },
  { "SRR_@res2",     OR,    GATE,  0x85, 0      , 0      , &l[54] , &l[59]          },
  { "SRR_@r2cl",     CLK,   CLCKL, 0x85, 0      , 0      , &l[53] , &l[60]          },

  { "DSR_",        IFUNCT,  GATE,  0x3f, &l[73] , &l[62] , &l[61] , 0               },	/* 29 pre-installed "DSR_" function block */
  { "DSR_@",         FF,    GATE,  0x8d, &l[61] , &l[64] , &l[62] , 0               },
  { "DSR_@expr",     OR,    GATE,  0x85, 0      , 0      , &l[68] , &l[73]          },
  { "DSR_@dcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[67] , &l[74]          },
  { "DSR_@set",      OR,    GATE,  0x85, 0      , 0      , &l[70] , &l[75]          },
  { "DSR_@scl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[69] , &l[76]          },
  { "DSR_@res",      OR,    GATE,  0x85, 0      , 0      , &l[72] , &l[77]          },
  { "DSR_@rcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[71] , &l[78]          },

  { "SH",          IFUNCT,  ARITH, 0x03, &l[85] , &l[80] , &l[79] , 0               },	/* 37 pre-installed "SH" function block */
  { "SH@",           SH,    ARITH, 0x8d, &l[79] , &l[82] , &l[80] , 0               },
  { "SH@dat",        ARN,   ARITH, 0x85, 0      , 0      , &l[84] , &l[85]          },
  { "SH@dcl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[83] , &l[86]          },

  { "SHR_",        IFUNCT,  ARITH, 0x0f, &l[96] , &l[88] , &l[87] , 0               },	/* 41 pre-installed "SHR_" function block */
  { "SHR_@",         SH,    ARITH, 0x8d, &l[87] , &l[90] , &l[88] , 0               },
  { "SHR_@dat",      ARN,   ARITH, 0x85, 0      , 0      , &l[93] , &l[96]          },
  { "SHR_@dcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[92] , &l[97]          },
  { "SHR_@res",      OR,    GATE,  0x85, 0      , 0      , &l[95] , &l[98]          },
  { "SHR_@rcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[94] , &l[99]          },

  { "SHSR_",       IFUNCT,  ARITH, 0x3f, &l[112], &l[101], &l[100], 0               },	/* 47 pre-installed "SHSR_" function block */
  { "SHSR_@",        SH,    ARITH, 0x8d, &l[100], &l[103], &l[101], 0               },
  { "SHSR_@dat",     ARN,   ARITH, 0x85, 0      , 0      , &l[107], &l[112]         },
  { "SHSR_@dcl",     CLK,   CLCKL, 0x85, 0      , 0      , &l[106], &l[113]         },
  { "SHSR_@set",     OR,    GATE,  0x85, 0      , 0      , &l[109], &l[114]         },
  { "SHSR_@scl",     CLK,   CLCKL, 0x85, 0      , 0      , &l[108], &l[115]         },
  { "SHSR_@res",     OR,    GATE,  0x85, 0      , 0      , &l[111], &l[116]         },
  { "SHSR_@rcl",     CLK,   CLCKL, 0x85, 0      , 0      , &l[110], &l[117]         },

  { "RISE",        IFUNCT,  GATE,  0x03, &l[124], &l[119], &l[118], 0               },	/* 55 pre-installed "RISE" function block */
  { "RISE@",         EF,    GATE,  0x8d, &l[118], &l[121], &l[119], 0               },
  { "RISE@trig",     OR,    GATE,  0x85, 0      , 0      , &l[123], &l[124]         },
  { "RISE@clk",      CLK,   CLCKL, 0x85, 0      , 0      , &l[122], &l[125]         },

  { "CHANGE",      IFUNCT,  GATE,  0x01, &l[132], &l[127], &l[126], 0               },	/* 59 pre-installed "CHANGE" function block */
  { "CHANGE@",       VF,    GATE,  0x8d, &l[126], &l[129], &l[127], 0               },
  { "CHANGE@trig",   OR,    GATE,  0x85, 0      , 0      , &l[131], &l[132]         },
  { "CHANGE@clk",    CLK,   CLCKL, 0x85, 0      , 0      , &l[130], &l[133]         },

  { "CHANGE2",     IFUNCT,  GATE,  0x02, &l[140], &l[135], &l[134], (List_e*)&b[59] },	/* 63 pre-installed "CHANGE2" alternate "CHANGE" */
  { "CHANGE2@",      VF,    GATE,  0x8d, &l[134], &l[137], &l[135], 0               },
  { "CHANGE2@val",   ARN,   ARITH, 0x85, 0      , 0      , &l[139], &l[140]         },
  { "CHANGE2@clk",   CLK,   CLCKL, 0x85, 0      , 0      , &l[138], &l[141]         },

  { "CLOCK",       IFUNCT,  CLCKL, 0x03, &l[148], &l[143], &l[142], 0               },	/* 67 pre-installed "CLOCK" function block */
  { "CLOCK@",        CLK,   CLCKL, 0x8d, &l[142], &l[145], &l[143], 0               },
  { "CLOCK@trig",    OR,    GATE,  0x85, 0      , 0      , &l[147], &l[148]         },
  { "CLOCK@clk",     CLK,   CLCKL, 0x85, 0      , 0      , &l[146], &l[149]         },

  { "CLOCK2",      IFUNCT,  CLCKL, 0x0f, &l[159], &l[151], &l[150], (List_e*)&b[67] },	/* 71 pre-installed "CLOCK2" alternate "CLOCK" */
  { "CLOCK2@",       CLK,   CLCKL, 0x8d, &l[150], &l[153], &l[151], 0               },
  { "CLOCK2@trig",   OR,    GATE,  0x85, 0      , 0      , &l[156], &l[159]         },
  { "CLOCK2@clk",    CLK,   CLCKL, 0x85, 0      , 0      , &l[155], &l[160]         },
  { "CLOCK2@trig2",  OR,    GATE,  0x85, 0      , 0      , &l[158], &l[161]         },
  { "CLOCK2@clk2",   CLK,   CLCKL, 0x85, 0      , 0      , &l[157], &l[162]         },

  { "TIMER",       IFUNCT,  TIMRL, 0x03, &l[169], &l[164], &l[163], 0               },	/* 77 pre-installed "TIMER" function block */
  { "TIMER@",        TIM,   TIMRL, 0x8d, &l[163], &l[166], &l[164], 0               },
  { "TIMER@trig",    OR,    GATE,  0x85, 0      , 0      , &l[168], &l[169]         },
  { "TIMER@clk",     CLK,   CLCKL, 0x85, 0      , 0      , &l[167], &l[170]         },

  { "TIMER02",     IFUNCT,  TIMRL, 0x0f, &l[180], &l[172], &l[171], (List_e*)&b[77] },	/* 81 pre-installed "TIMER02" alternate "TIMER" */
  { "TIMER02@",      TIM,   TIMRL, 0x8d, &l[171], &l[174], &l[172], 0               },
  { "TIMER02@trig",  OR,    GATE,  0x85, 0      , 0      , &l[177], &l[180]         },
  { "TIMER02@clk",   CLK,   CLCKL, 0x85, 0      , 0      , &l[176], &l[181]         },
  { "TIMER02@trig2", OR,    GATE,  0x85, 0      , 0      , &l[179], &l[182]         },
  { "TIMER02@clk2",  CLK,   CLCKL, 0x85, 0      , 0      , &l[178], &l[183]         },

  { "TIMER1",      IFUNCT,  TIMRL, 0x03, &l[190], &l[185], &l[184], 0               },	/* 87 pre-installed "TIMER1" function block */
  { "TIMER1@",       TIM,   TIMRL, 0x8d, &l[184], &l[187], &l[185], 0               },
  { "TIMER1@trig",   OR,    GATE,  0x85, 0      , 0      , &l[189], &l[190]         },
  { "TIMER1@clk",    CLK,   CLCKL, 0x85, 0      , 0      , &l[188], &l[191]         },

  { "TIMER12",     IFUNCT,  TIMRL, 0x0f, &l[201], &l[193], &l[192], (List_e*)&b[87] },	/* 91 pre-installed "TIMER12" alternate "TIMER1" */
  { "TIMER12@",      TIM,   TIMRL, 0x8d, &l[192], &l[195], &l[193], 0               },
  { "TIMER12@trig",  OR,    GATE,  0x85, 0      , 0      , &l[198], &l[201]         },
  { "TIMER12@clk",   CLK,   CLCKL, 0x85, 0      , 0      , &l[197], &l[202]         },
  { "TIMER12@trig2", OR,    GATE,  0x85, 0      , 0      , &l[200], &l[203]         },
  { "TIMER12@clk2",  CLK,   CLCKL, 0x85, 0      , 0      , &l[199], &l[204]         },

  { "SR",          IFUNCT,  GATE,  0x0f, &l[214], &l[206], &l[205], 0               },	/* 97 pre-installed "SR" function block */
  { "SR@",           FF,    GATE,  0x8d, &l[205], &l[208], &l[206], 0               },
  { "SR@set",        OR,    GATE,  0x85, 0      , 0      , &l[211], &l[214]         },
  { "SR@scl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[210], &l[215]         },
  { "SR@res",        OR,    GATE,  0x85, 0      , 0      , &l[213], &l[216]         },
  { "SR@rcl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[212], &l[217]         },

  { "SRX",         IFUNCT,  GATE,  0x0f, &l[229], &l[219], &l[218], 0               },	/* 103 pre-installed "SRX" function block */
  { "SRX@",          FF,    GATE,  0x8d, &l[218], &l[221], &l[219], 0               },
  { "SRX@set",       OR,    GATE,  0x86, 0      , 0      , &l[227], &l[229]         },
  { "SRX@scl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[223], &l[230]         },
  { "SRX@res",       OR,    GATE,  0x86, 0      , 0      , &l[228], &l[231]         },
  { "SRX@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[226], &l[232]         },

  { "JK",          IFUNCT,  GATE,  0x0f, &l[244], &l[234], &l[233], 0               },	/* 109 pre-installed "JK" function block */
  { "JK@",           FF,    GATE,  0x8e, &l[233], &l[236], &l[234], &l[242]         },
  { "JK@set",        OR,    GATE,  0x85, 0      , 0      , &l[240], &l[244]         },
  { "JK@scl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[238], &l[245]         },
  { "JK@res",        OR,    GATE,  0x85, 0      , 0      , &l[243], &l[246]         },
  { "JK@rcl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[241], &l[247]         },

  { "SRR",         IFUNCT,  GATE,  0x3f, &l[260], &l[249], &l[248], 0               },	/* 115 pre-installed "SRR" function block */
  { "SRR@",          FF,    GATE,  0x8d, &l[248], &l[251], &l[249], 0               },
  { "SRR@set",       OR,    GATE,  0x85, 0      , 0      , &l[255], &l[260]         },
  { "SRR@scl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[254], &l[261]         },
  { "SRR@res",       OR,    GATE,  0x85, 0      , 0      , &l[257], &l[262]         },
  { "SRR@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[256], &l[263]         },
  { "SRR@res2",      OR,    GATE,  0x85, 0      , 0      , &l[259], &l[264]         },
  { "SRR@r2cl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[258], &l[265]         },

  { "DR",          IFUNCT,  GATE,  0x0f, &l[280], &l[267], &l[266], 0               },	/* 123 pre-installed "DR" function block */
  { "DR@",           FF,    GATE,  0x8e, &l[266], &l[269], &l[267], &l[273]         },
  { "DR@dat",        OR,    GATE,  0x85, 0      , 0      , &l[271], &l[280]         },
  { "DR@dcl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[275], &l[281]         },
  { "DR@res",        OR,    GATE,  0x86, 0      , 0      , &l[279], &l[282]         },
  { "DR@rcl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[278], &l[283]         },

  { "DSR@i",         OR,    GATE,  0x8a, 0      , &l[289], &l[297], 0               },
  { "DSR",         IFUNCT,  GATE,  0x3f, &l[307], &l[284], &l[286], 0               },	/* 130 pre-installed "DSR" function block */
  { "DSR@",          FF,    GATE,  0x8e, &l[286], &l[291], &l[287], &l[296]         },
  { "DSR@dat",       OR,    GATE,  0x85, 0      , 0      , &l[294], &l[307]         },
  { "DSR@dcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[298], &l[308]         },
  { "DSR@set",       OR,    GATE,  0x86, 0      , 0      , &l[305], &l[309]         },
  { "DSR@scl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[301], &l[310]         },
  { "DSR@res",       OR,    GATE,  0x86, 0      , 0      , &l[306], &l[311]         },
  { "DSR@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[304], &l[312]         },

  { "SHR",         IFUNCT,  ARITH, 0x0f, &l[325], &l[314], &l[313], 0               },	/* 138 pre-installed "SHR" function block */
  { "SHR@",          SH,    ARITH, 0x8e, &l[313], &l[316], &l[314], &l[321]         },
  { "SHR@dat",       ARN,   ARITH, 0x85, 0      , 0      , &l[322], &l[325]         },
  { "SHR@dcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[319], &l[326]         },
  { "SHR@res",       OR,    GATE,  0x86, 0      , 0      , &l[324], &l[327]         },
  { "SHR@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[323], &l[328]         },

  { "SHSR",        IFUNCT,  ARITH, 0x3f, &l[347], &l[330], &l[329], 0               },	/* 144 pre-installed "SHSR" function block */
  { "SHSR@",         SH,    ARITH, 0x8e, &l[329], &l[332], &l[330], &l[339]         },
  { "SHSR@dat",      ARN,   ARITH, 0x85, 0      , 0      , &l[340], &l[347]         },
  { "SHSR@dcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[337], &l[348]         },
  { "SHSR@set",      OR,    GATE,  0x86, 0      , 0      , &l[345], &l[349]         },
  { "SHSR@scl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[341], &l[350]         },
  { "SHSR@res",      OR,    GATE,  0x86, 0      , 0      , &l[346], &l[351]         },
  { "SHSR@rcl",      CLK,   CLCKL, 0x85, 0      , 0      , &l[344], &l[352]         },

  { "ST",          IFUNCT,  GATE,  0x03, &l[362], &l[354], &l[353], 0               },	/* 152 pre-installed "ST" function block */
  { "ST@",           FF,    GATE,  0x8e, &l[353], &l[356], &l[354], &l[361]         },
  { "ST@set",        OR,    GATE,  0x85, 0      , 0      , &l[359], &l[362]         },
  { "ST@scl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[358], &l[363]         },
  { "ST@tcl",        CLK,   CLCKL, 0x85, 0      , 0      , &l[360], &l[364]         },

  { "SRT",         IFUNCT,  GATE,  0x0f, &l[377], &l[366], &l[365], 0               },	/* 157 pre-installed "SRT" function block */
  { "SRT@",          FF,    GATE,  0x8e, &l[365], &l[368], &l[366], &l[376]         },
  { "SRT@set",       OR,    GATE,  0x85, 0      , 0      , &l[372], &l[377]         },
  { "SRT@scl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[371], &l[378]         },
  { "SRT@res",       OR,    GATE,  0x85, 0      , 0      , &l[374], &l[379]         },
  { "SRT@rcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[373], &l[380]         },
  { "SRT@tcl",       CLK,   CLCKL, 0x85, 0      , 0      , &l[375], &l[381]         },

  { "FALL",        IFUNCT,  GATE,  0x03, &l[388], &l[383], &l[382], 0               },	/* 164 pre-installed "FALL" function block */
  { "FALL@",         EF,    GATE,  0x8d, &l[382], &l[385], &l[383], 0               },
  { "FALL@fall",     OR,    GATE,  0x85, 0      , 0      , &l[387], &l[388]         },
  { "FALL@clk",      CLK,   CLCKL, 0x85, 0      , 0      , &l[386], &l[389]         },

  { "LATCH",       IFUNCT,  GATE,  0x0f, &l[396], &l[391], &l[390], 0               },	/* 168 pre-installed "LATCH" function block */
  { "LATCH@",        LATCH, GATE,  0x8e, &l[390], &l[393], &l[391], &l[393]         },
  { "LATCH@set",     OR,    GATE,  0x85, 0      , 0      , &l[394], &l[396]         },
  { "LATCH@res",     OR,    GATE,  0x85, 0      , 0      , &l[395], &l[397]         },

  { "DLATCH",      IFUNCT,  GATE,  0x0f, &l[406], &l[399], &l[398], 0               },	/* 172 pre-installed "DLATCH" function block */
  { "DLATCH@",       FF,    GATE,  0x8e, &l[398], &l[401], &l[399], &l[403]         },
  { "DLATCH@set",    OR,    GATE,  0x85, 0      , 0      , &l[404], &l[406]         },
  { "DLATCH@res",    OR,    GATE,  0x85, 0      , 0      , &l[405], &l[407]         },
  { "DLATCH@clk",    CLK,   CLCKL, 0x85, 0      , 0      , &l[402], &l[408]         },

											/* 177 pre-installed BuiltIn function Symbols */
#else	/* BOOT_COMPILE */
  /* name	     type   ftype  uVal	*/
  { "FORCE",	     KEYW,  0,     BFORCE, 0    , 0      , 0      , 0 },	/* FORCE function - generates LATCH */
  { "D",	     KEYW,  D_FF,  BLTIN1, 0    , 0      , 0      , 0 },	/* D flip-flop */
  { "SR_",	     KEYW,  S_FF,  BLTIN2, 0    , 0      , 0      , 0 },	/* SR flip-flop with simple set/reset */
  { "DR_",	     KEYW,  D_FF,  BLTIN2, 0    , 0      , 0      , 0 },	/* D flip-flop with simple reset */
  { "SRR_",	     KEYW,  S_FF,  BLTIN3, 0    , 0      , 0      , 0 },	/* SRR flip-flop with simple set/2xreset */
  { "DSR_",	     KEYW,  D_FF,  BLTIN3, 0    , 0      , 0      , 0 },	/* D flip-flop with simple set/reset */
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
  { "strict",	     KEYW,  1,     USETYPE,0    , 0      , 0      , 0 },	/* PRAGMA - MAXUSETYPE 2 */
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
	/* no name may occurr twice - otherwise install fails */
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
#ifdef	WIN32
	    " # line %d \"%[-/:A-Za-z_.0-9<>]\"	%s",
#else	/* not WIN32 */
	    " # line %d \"%[-/A-Za-z_.0-9<>]\"	%s",
#endif	/* WIN32 */
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

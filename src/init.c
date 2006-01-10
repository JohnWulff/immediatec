static const char init_c[] =
"@(#)$Id: init.c,v 1.30 2005/10/21 18:51:44 jw Exp $";
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
 *	init.c
 *	parallel plc - initialise compiler symbol table with keywords
 *
 *******************************************************************/

#ifdef _WINDOWS
#include	<windows.h>
#endif
#include	<stdio.h>
#include	"icc.h"
#include	"comp.h"
#include	"comp_tab.h"

Symbol *	iclock;			/* default clock */

/********************************************************************
 *
 *	Initialise Symbol table
 *
 *		reserved words:		type KEYW
 *					uVal = compiler_token
 *					ftype used in compilation
 *		built-in symbols:	type for built-in
 *					ftype for built-in
 *
 *******************************************************************/

struct bi {
    char *		name;
    unsigned char	type;
    int			uVal;		/* yacc token for type KEYW */
    unsigned char	ftype;
};

/********************************************************************
 *
 *  Old built in symbols for iC used if calling compiler with -d12000
 *
 *  these are now handled by pre-defined built in function blocks
 *  later in this source.
 *
 *******************************************************************/

static struct bi builtinsOld[] = {
    /* name	type	uVal	ftype */
  { "DR",	KEYW,	BLTIN2,	D_FF,	}, /* D flip-flop with reset */
  { "DSR",	KEYW,	BLTIN3,	D_FF,	}, /* D flip-flop with set/reset */
  { "SHR",	KEYW,	BLTIN2,	D_SH,	}, /* sample and hold with reset */
  { "SHSR",	KEYW,	BLTIN3,	D_SH,	}, /* sample and hold with set/reset */
#ifdef SR_X 
  /*
   * define here if SR(set, res) is cross checked in initialFunctions
   * SRX(set, res) always has this feature
   * cross checked SR is not the default because of extra code generated
   * which is normally not needed - SRX is provided if it is required
   */
  { "SR",	KEYW,	BLTINX,	S_FF,	}, /* R_FF for reset master */
#endif
  { "SRX",	KEYW,	BLTINX,	S_FF,	}, /* R_FF for reset master */
  { "JK",	KEYW,	BLTINJ,	S_FF,	}, /* R_FF for reset master */
  { "LATCH",	KEYW,	BLATCH,	0,	},
  { "DLATCH",	KEYW,	DLATCH,	D_FF,	},
  { 0,		0,	0,	0,	},
};

/********************************************************************
 *
 *  All standard built in symbols for iC
 *
 *  extra C keywords cause a syntax error if used as immediate variables.
 *  The erroneous line in the iC code is marked with a pointer to the
 *  offending keyword.
 *
 *  Otherwise the C compiler would flag them with a syntax error message
 *  referring to the line number of the generated Gate in the C code
 *  produced by th iC compiler, which is harder to trace back to the source.
 *
 *  C keywords int if else extern return switch and void used in iC cause
 *  similar iC syntax errors if used as immediate variable names.
 *
 *******************************************************************/

static struct bi builtins[] = {
    /* name	type	uVal	ftype */
  { "D",	KEYW,	BLTIN1,	D_FF,	}, /* D flip-flop */
  { "DR_",	KEYW,	BLTIN2,	D_FF,	}, /* D flip-flop with simple reset */
  { "DSR_",	KEYW,	BLTIN3,	D_FF,	}, /* D flip-flop with simple set/reset */
  { "SH",	KEYW,	BLTIN1,	D_SH,	}, /* sample and hold */
  { "SHR_",	KEYW,	BLTIN2,	D_SH,	}, /* sample and hold with simple reset */
  { "SHSR_",	KEYW,	BLTIN3,	D_SH,	}, /* sample and hold with simple set/reset */
  { "CHANGE",	KEYW,	BLTIN1,	CH_BIT,	}, /* pulse on anlog or digital change */
  { "RISE",	KEYW,	BLTIN1,	RI_BIT,	}, /* pulse on digital rising edge */
#ifndef SR_X 
  { "SR",	KEYW,	BLTIN2,	S_FF,	}, /* R_FF for reset master */
#endif
  { "SR_",	KEYW,	BLTIN2,	S_FF,	}, /* R_FF for reset master */
  { "SRT",	KEYW,	BLTINT,	S_FF,	}, /* monoflop with timed reset*/
  { "if",	KEYW,	IF,	F_CF,	},
  { "else",	KEYW,	ELSE,	F_CE,	},
  { "switch",	KEYW,	SWITCH,	F_SW,	},
  { "CLOCK",	KEYW,	CBLTIN,	CLCK,	},
  { "TIMER",	KEYW,	TBLTIN,	TIMR,	}, /* normal timer with preset off 0 */
  { "TIMER1",	KEYW,	TBLTI1,	TIMR,	}, /* alternate timer with preset off 1 */
  { "FORCE",	KEYW,	BFORCE,	0,	},
  { "extern",	KEYW,	EXTERN,	0,	},
  { "assign",	KEYW,	ASSIGN,	0,	},
  { "return",	KEYW,	RETURN,	0,	},
  { "use",	KEYW,	USE,	1,	}, /* turn on use */
  { "no",	KEYW,	USE,	0,	}, /* turn off use */
  { "alias",	KEYW,	USETYPE, 0,	}, /* check that no more than MAXUSETYPE USETYPE's occurr */
  { "strict",	KEYW,	USETYPE, 1,	}, /* MAXUSETYPE 2 */
  { "imm",	KEYW,	IMM,	0,	},
  { "immC",	KEYW,	IMM,	1,	},
  { "void",	KEYW,	VOID,	UDFA,	},
  { "bit",	KEYW,	TYPE,	GATE,	},
  { "int",	KEYW,	TYPE,	ARITH,	},
  { "clock",	KEYW,	TYPE,	CLCKL,	},
  { "timer",	KEYW,	TYPE,	TIMRL,	},
  { "this",	KEYW,	LEXERR,	0,	}, /* only used in function block definitions */
  { "auto",	KEYW,	LEXERR,	0,	}, /* C keywords to cause syntax errors if used in iC */
  { "break",	KEYW,	LEXERR,	0,	},
  { "case",	KEYW,	LEXERR,	0,	},
  { "char",	KEYW,	LEXERR,	0,	},
  { "const",	KEYW,	LEXERR,	0,	},
  { "continue",	KEYW,	LEXERR,	0,	},
  { "default",	KEYW,	LEXERR,	0,	},
  { "do",	KEYW,	LEXERR,	0,	},
  { "double",	KEYW,	LEXERR,	0,	},
  { "enum",	KEYW,	LEXERR,	0,	},
  { "float",	KEYW,	LEXERR,	0,	},
  { "for",	KEYW,	LEXERR,	0,	},
  { "goto",	KEYW,	LEXERR,	0,	},
  { "long",	KEYW,	LEXERR,	0,	},
  { "register",	KEYW,	LEXERR,	0,	},
  { "short",	KEYW,	LEXERR,	0,	},
  { "signed",	KEYW,	LEXERR,	0,	},
  { "sizeof",	KEYW,	LEXERR,	0,	},
  { "static",	KEYW,	LEXERR,	0,	},
  { "struct",	KEYW,	LEXERR,	0,	},
  { "typedef",	KEYW,	LEXERR,	0,	},
  { "union",	KEYW,	LEXERR,	0,	},
  { "unsigned",	KEYW,	LEXERR,	0,	},
  { "volatile",	KEYW,	LEXERR,	0,	},
  { "while",	KEYW,	LEXERR,	0,	},
  { "fortran",	KEYW,	LEXERR,	0,	},
  { "asm",	KEYW,	LEXERR,	0,	},
  { "iC_Gt",	CTYPE,	YYERRCODE, 0,	}, /* initial Gate C-type from icg.h */
  { ICONST,	NCONST, 0,	ARITH,	}, /* iConst Symbol */
  { "iClock",	CLK,	0,	CLCKL,	}, /* must be last non-zero entry */
  { 0,		0,	0,	0,	},
};

/********************************************************************
 *
 *	iC system function definitions
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
 *	Old style imm functions without these definitions use -d20000
 *
 *******************************************************************/

const char initialFunctions[] = "\
/* iC system function definitions */\n\
"
#ifdef SR_X 
"\
imm bit SR(bit set, clock scl, bit res, clock rcl) {\n\
    return SR_(set & ~res, scl, ~set & res, rcl); }\n\
"
#endif
"\
imm bit SRX(bit set, clock scl, bit res, clock rcl) {\n\
    return SR_(set & ~res, scl, ~set & res, rcl); }\n\
imm bit JK(bit set, clock scl, bit res, clock rcl) {\n\
    return SR_(~this & set, scl, this & res, rcl); }\n\
imm bit DR(bit dat, clock dcl, bit res, clock rcl) {\n\
    return DR_(dat & ~res | this & res, dcl, res, rcl); }\n\
imm bit DSR(bit dat, clock dcl, bit set, clock scl, bit res, clock rcl) {\n\
    imm bit i = set | res;\n\
    return DSR_(dat & ~i | this & i, dcl, set & ~res, scl, ~set & res, rcl); }\n\
imm int SHR(int dat, clock dcl, bit res, clock rcl) {\n\
    return SHR_( res ? this : dat, dcl, res, rcl); }	/* line 202 */\n\
imm int SHSR(int dat, clock dcl, bit set, clock scl, bit res, clock rcl) {\n\
    return SHSR_(set | res ? this : dat, dcl, set & ~res, scl, ~set & res, rcl); }\n\
imm bit LATCH(bit set, bit res) {\n\
    return FORCE(this, set, res); }\n\
imm bit DLATCH(bit set, bit res, clock clk) {\n\
    return D(FORCE(this, set, res), clk); }\n\
/* end iC system function definitions */\n\
";

/********************************************************************
 *
 *  The following function blocks generate C functions 1 and 2
 *  *** adjust line numbers to point to correct line in initialFunctions
 *  *** comment string is required in comp.y line 2538
 *  *** tab before comment is required in comp.y line 2586
 *  *** adjust GEN_COUNT in comp.h to reflect correct number (used in comp.y)
 *
 *  genName and genLineNums are used to identify function lines in get()
 *  generated with genLines.
 *
 *******************************************************************/

const char *	genLines[]  = {
    "",
    "#line 202 \"init.c\"	/* in pre-compiled function block SHR */",
    "#line 204 \"init.c\"	/* in pre-compiled function block SHSR */",
};

const char *	genName = "init.c";	/* must be the same as above */

int		genLineNums[] = {
    0,
    202,				/* must be the same as above */
    204,
};

void
init(void)				/* install constants and built-ins */
{
    int		io;
    int		instanceNum;
    Symbol *	sp = 0;
    FILE *	H1p;
    char	funName[BUFS];
    char	lineBuf[BUFS];

    if ((iC_debug & 010000) && inpFN == 0) {
	fprintf(iC_outFP, "initialFunctions:\n%s\n", initialFunctions);
    }
    if (iC_debug & 020000) {
	for (io = 0; builtinsOld[io].name; io++) {
	    sp = install(builtinsOld[io].name, builtinsOld[io].type, builtinsOld[io].ftype);
	    sp->u_val = builtinsOld[io].uVal;	/* set u_val in Symbol just installed */
	}
    }
    for (io = 0; builtins[io].name; io++) {
	/* no name may occurr twice - otherwise install fails */
	sp = install(builtins[io].name, builtins[io].type, builtins[io].ftype);
	sp->u_val = builtins[io].uVal;		/* set u_val in Symbol just installed */
    }
    iclock = sp;	/* 'iclock' with name "iClock" is default clock when loop finishes */

    if (iC_aflag != 0) {		/* build several files to be linked */
	if ((H1p = fopen(H1name, "r")) != 0) {			/* scan _list1.h */
	    while (fgets(lineBuf, sizeof lineBuf, H1p)) {	/* read previous instance numbers */
		if (sscanf(lineBuf, "/* %s\t%d */", funName, &instanceNum) == 2) {
		    if ((sp = lookup(funName)) == 0) {
			sp = install(funName, IFUNCT, UDFA);	/* function not defined yet */
		    }
		    sp->u_val = instanceNum;		/* initialise instance number */
		}
	    }
	    fclose(H1p);
	    /* if a previous _list1.h exists, it will be extended ZZZ */
	} else {
	    iC_lflag = 1;			/* need to write #define in new _list1.h */
	}
    }
}

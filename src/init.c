static const char init_c[] =
"@(#)$Id: init.c,v 1.24 2004/02/21 17:29:11 jw Exp $";
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
 *	init.c
 *	parallel plc - initialise compiler symbol table with keywords
 *
 *******************************************************************/

#ifdef _WINDOWS
#include	<windows.h>
#endif
#include	<stdio.h>
#include	"icg.h"
#include	"icc.h"
#include	"comp.h"
#include	"comp_tab.h"

Symbol *	iclock;		/* default clock */

/********************************************************************
 *
 *	Initialise Symbol table
 *
 *		reserved words:		type KEYW
 *					uVal = compiler_token
 *					ftype used in compilation
 *
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

static struct bi builtinsOld[] = {
    /* name	type	uVal	ftype */
  { "DR",	KEYW,	BLTIN2,	D_FF,	}, /* D flip-flop with reset */
  { "DSR",	KEYW,	BLTIN3,	D_FF,	}, /* D flip-flop with set/reset */
//{ "SR",	KEYW,	BLTIN2,	S_FF,	}, /* R_FF for reset master */
  { "JK",	KEYW,	BLTINJ,	S_FF,	}, /* R_FF for reset master */
  { "L",	KEYW,	BLATCH,	0,	},
  { "LATCH",	KEYW,	BLATCH,	0,	},
  { "DL",	KEYW,	DLATCH,	D_FF,	},
  { "DLATCH",	KEYW,	DLATCH,	D_FF,	},
  { 0,		0,	0,	0,	},
};

static struct bi builtins[] = {
    /* name	type	uVal	ftype */
  { "D",	KEYW,	BLTIN1,	D_FF,	}, /* D flip-flop */
  { "DR_",	KEYW,	BLTIN2,	D_FF,	}, /* D flip-flop with simple reset */
  { "DSR_",	KEYW,	BLTIN3,	D_FF,	}, /* D flip-flop with simple set/reset */
  { "SH",	KEYW,	BLTIN1,	D_SH,	}, /* sample and hold */
  { "SHR",	KEYW,	BLTIN2,	D_SH,	}, /* sample and hold with reset */
  { "SHSR",	KEYW,	BLTIN3,	D_SH,	}, /* sample and hold with set/reset */
  { "CHANGE",	KEYW,	BLTIN1,	CH_BIT,	}, /* pulse on anlog or digital change */
  { "RISE",	KEYW,	BLTIN1,	RI_BIT,	}, /* pulse on digital rising edge */
  { "SR",	KEYW,	BLTIN2,	S_FF,	}, /* R_FF for reset master */
  { "SR_",	KEYW,	BLTIN2,	S_FF,	}, /* R_FF for reset master */
  { "SRT",	KEYW,	BLTINT,	S_FF,	}, /* monoflop with timed reset*/
  { "IF",	KEYW,	IF,	F_CF,	},
  { "if",	KEYW,	IF,	F_CF,	},
  { "ELSE",	KEYW,	ELSE,	F_CE,	},
  { "else",	KEYW,	ELSE,	F_CE,	},
  { "SWITCH",	KEYW,	SWITCH,	F_SW,	},
  { "switch",	KEYW,	SWITCH,	F_SW,	},
  { "C",	KEYW,	CBLTIN,	CLCK,	},
  { "CLOCK",	KEYW,	CBLTIN,	CLCK,	},
  { "T",	KEYW,	TBLTIN,	TIMR,	}, /* there is no default timer */
  { "TIMER",	KEYW,	TBLTIN,	TIMR,	}, /* normal timer with preset off 0 */
  { "T1",	KEYW,	TBLTI1,	TIMR,	}, /* alternate timer with preset off 1 */
  { "TIMER1",	KEYW,	TBLTI1,	TIMR,	}, /* alternate timer with preset off 1 */
  { "F",	KEYW,	BFORCE,	0,	},
  { "FORCE",	KEYW,	BFORCE,	0,	},
  { "extern",	KEYW,	EXTERN,	0,	},
  { "assign",	KEYW,	ASSIGN,	0,	},
  { "return",	KEYW,	RETURN,	0,	},
  { "imm",	KEYW,	IMM,	0,	},
  { "void",	KEYW,	VOID,	UDFA,	},
  { "bit",	KEYW,	TYPE,	GATE,	},
  { "int",	KEYW,	TYPE,	ARITH,	},
  { "clock",	KEYW,	TYPE,	CLCKL,	},
  { "timer",	KEYW,	TYPE,	TIMRL,	},
  { "Gate",	CTYPE,	YYERRCODE, 0,	}, /* initial C type from icc.h */
  { "iClock",	CLK,	0,	CLCKL,	}, /* must be last non-zero entry */
  { 0,		0,	0,	0,	},
};

void
init(void)		/* install constants and built-ins */
{
    int		io;
    int		instanceNum;
    Symbol *	sp;
    FILE *	H1p;
    char	funName[BUFS];
    char	lineBuf[BUFS];

    if (debug & 020000) {
	for (io = 0; builtinsOld[io].name; io++) {
	    sp = install(builtinsOld[io].name, builtinsOld[io].type, builtinsOld[io].ftype);
	    sp->u_val = builtinsOld[io].uVal;	/* set u_val in Symbol just installed */
	}
    }
    for (io = 0; builtins[io].name; io++) {
	/* no name may occurr twice - otherwise install fails */
	sp = install(builtins[io].name, builtins[io].type, builtins[io].ftype);
	sp->u_val = builtins[io].uVal;	/* set u_val in Symbol just installed */
    }
    iclock = sp;	/* 'iclock' with name "iClock" is default clock when loop finishes */
    if (aflag && (H1p = fopen(H1name, "r")) != 0) {	/* list _list_.c */
	while (fgets(lineBuf, sizeof lineBuf, H1p)) {	/* read previous instance numbers */
	    if (sscanf(lineBuf, "/* %s\t%d */", funName, &instanceNum) == 2) {
		if ((sp = lookup(funName)) == 0) {
		    sp = install(funName, IFUNCT, UDFA); /* function not defined yet */;
		}
		sp->u_val = instanceNum;		/* initialise instance number */
	    }
	}
	fclose(H1p);
    }
}

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
 *	A listing of these functions can be generated with -d10044
 *	Old style imm functions without these definitions use -d20000
 *
 *******************************************************************/

const char initialFunctions[] = "\
/* iC system function definitions */\n\
imm bit SRX(bit set, clock scl, bit res, clock rcl) {\n\
    return SR_(set & ~res, scl, ~set & res, rcl); }\n\
imm bit JK(bit set, clock scl, bit res, clock rcl) {\n\
    return SR_(~this & set, scl, this & res, rcl); }\n\
imm bit DR(bit dat, clock dcl, bit res, clock rcl) {\n\
    return DR_(dat & ~res | this & res, dcl, res, rcl); }\n\
imm bit DSR(bit dat, clock dcl, bit set, clock scl, bit res, clock rcl) {\n\
    imm bit i = set | res;\n\
    return DSR_(dat & ~i | this & i, dcl, set & ~res, scl, ~set & res, rcl); }\n\
imm bit L(bit set, bit res) {\n\
    return F(this, set, res); }\n\
imm bit DL(bit set, bit res, clock clk) {\n\
    return D(F(this, set, res), clk); }\n\
imm bit LATCH(bit s, bit r) {\n\
    return L(s, r); }\n\
imm bit DLATCH(bit s, bit r, clock c) {\n\
    return DL(s, r, c); }\n\
/* end iC system function definitions */\n\
";

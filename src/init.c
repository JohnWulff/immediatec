static const char init_c[] =
"@(#)$Id: init.c,v 1.18 2002/07/01 09:50:53 jw Exp $";
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
#include	"icc.h"
#include	"comp.h"
#include	"y.tab.h"

Symbol *	iclock;		/* default clock */

/********************************************************************
 *
 *	Initialise Symbol table
 *
 *		reserved words:		type KEYW
 *					u.val = compiler_token
 *					ftype used in compilation
 *
 *		built-in symbols:	type for built-in
 *					ftype for built-in
 *
 *******************************************************************/

static struct {
    char *		name;
    unsigned char	type;
    int			u_val;		/* yacc token for type KEYW */
    unsigned char	ftype;
} builtins[] = {
    /* name	type	u_val	ftype */
  { "D",	KEYW,	BLTIN1,	D_FF,	}, /* D flip-flop */
  { "DR",	KEYW,	BLTIN2,	D_FF,	}, /* D flip-flop with reset */
  { "DSR",	KEYW,	BLTIN3,	D_FF,	}, /* D flip-flop with set/reset */
  { "SH",	KEYW,	BLTIN1,	D_SH,	}, /* sample and hold */
  { "SHR",	KEYW,	BLTIN2,	D_SH,	}, /* sample and hold with reset */
  { "SHSR",	KEYW,	BLTIN3,	D_SH,	}, /* sample and hold with set/reset */
  { "CHANGE",	KEYW,	BLTIN1,	CH_BIT,	}, /* pulse on anlog or digital change */
  { "RISE",	KEYW,	BLTIN1,	RI_BIT,	}, /* pulse on digital rising edge */
  { "SR",	KEYW,	BLTIN2,	S_FF,	}, /* R_FF for reset master */
  { "JK",	KEYW,	BLTINJ,	S_FF,	}, /* R_FF for reset master */
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
  { "L",	KEYW,	BLATCH,	0,	},
  { "LATCH",	KEYW,	BLATCH,	0,	},
  { "DL",	KEYW,	DLATCH,	D_FF,	},
  { "DLATCH",	KEYW,	DLATCH,	D_FF,	},
  { "extern",	KEYW,	EXTERN,	0,	},
  { "imm",	KEYW,	IMM,	0,	},
  { "bit",	KEYW,	TYPE,	GATE,	},
  { "clock",	KEYW,	TYPE,	CLCKL,	},
  { "int",	KEYW,	TYPE,	ARITH,	},
  { "timer",	KEYW,	TYPE,	TIMRL,	},
  { "iClock",	CLK,	0,	CLCKL,	}, /* must be last non-zero entry */
  { 0,		0,	0,	0,	},
};

void
init(void)		/* install constants and built-ins */
{
    int		io;

    for (io = 0; builtins[io].name; io++) {
	iclock = install(builtins[io].name, builtins[io].type, builtins[io].ftype);
	iclock->u.val = builtins[io].u_val;/* set u.val in Symbol just installed */
    }
    /* 'iclock' with name "iClock" is default clock when loop finishes */
}
